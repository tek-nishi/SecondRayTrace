﻿
#pragma once

//
// パストレース
//

#include "defines.hpp"
#include <limits>
#include "collision.hpp"
#include "random.hpp"
#include "qmc.hpp"
#include "bvh.hpp"
#include "hdri.hpp"


namespace Pathtrace {

struct TestInfo {
  Vec3f hit_pos;
  Vec3f hit_normal;
  Vec3f hit_uv;
  
  const Material* material;
};




Vec3f radiationVector_qmc(const Vec3f& w, Qmc& random) {
  Vec3f u = (std::abs(w.x()) > 0.0001) ? Vec3f::UnitY().cross(w).normalized()
                                       : Vec3f::UnitX().cross(w).normalized();
  Vec3f v = w.cross(u);

  const Real r1  = 2.0 * M_PI * random.next();
  const Real r2  = random.next();
  const Real r2s = std::sqrt(r2);
  
  return (u * std::cos(r1) * r2s
        + v * std::sin(r1) * r2s
        + w * std::sqrt(1.0 - r2)).normalized();
}


// 光線とオブジェクトの交差判定
std::pair<bool, TestInfo> testRayObj(const Vec3f& ray_start, const Vec3f& ray_vec,
                                     bool back_face,
                                     const Model& model) {
  TestInfo info;

  bool hit           = false;
  Real hit_t_current = std::numeric_limits<Real>::max();

  const Triangle* normal_current;
  const Triangle* uv_current;
  Vec3f hit_uvw_current;

  const auto& meshes = model.mesh();
  for (const auto& mesh : meshes) {
    // AABBでざっくり判定
    Vec3f bbox_hit;
    Real bbox_hit_t;
    if (!testRayAABB(bbox_hit, bbox_hit_t, ray_start, ray_vec, mesh->bbox())) {
      continue;
    }
    if (bbox_hit_t > hit_t_current) continue;
        
    // Mesh内のPolygonと交差判定
    const auto& polygons = mesh->polygons();
    const auto& normals  = mesh->normals();
    const auto& uvs      = mesh->uvs();

    for (int ip = 0; ip < polygons.size(); ++ip) {
      Vec3f hit_pos;
      Real hit_t;
      Vec3f hit_n;
      Vec3f hit_c;
      if (testRayTriangle(hit_pos, hit_t, hit_n, hit_c,
                          ray_start, ray_vec,
                          polygons[ip], back_face)) {
        hit = true;

        if (hit_t < hit_t_current) {
          hit_t_current = hit_t;

          info.hit_pos  = hit_pos;
          info.material = &model.material()[mesh->materialIndex()];

          hit_uvw_current = hit_c;
          normal_current  = &normals[ip];
          if (info.material->hasTexture()) uv_current = &uvs[ip];
        }
      }
    }
  }

  if (hit) {
    // TIPS:面法線は頂点の法線と重心座標から求められる
    info.hit_normal = (normal_current->a * hit_uvw_current.x()
                     + normal_current->b * hit_uvw_current.y()
                     + normal_current->c * hit_uvw_current.z()).normalized();

    if (info.material->hasTexture()) {
      // TIPS:UV座標も重心座標から求められる
      info.hit_uv = uv_current->a * hit_uvw_current.x()
                  + uv_current->b * hit_uvw_current.y()
                  + uv_current->c * hit_uvw_current.z();
    }
  }
  
  return std::make_pair(hit, info);
}


// 「薄いレンズ」内の位置を乱数で決める
// ※一様分布ではない
Vec2f concentricSampleDisk(const Real u1, const Real u2) {
  // [0, 1]の一様乱数u1,u2を[-1, 1]の一様乱数sx,syに写像
  const Real sx = 2.0 * u1 - 1;
  const Real sy = 2.0 * u2 - 1;

  // sx, syが0,0だった場合は特別に処理
  if (sx == 0.0 && sy == 0.0) {
    return Vec2f::Zero();
  }

  Real r, theta;
	// 四つに分割した円の各部位で別々の処理になる
  if (sx >= -sy) {
    if (sx > sy) {
      r = sx;
      if (sy > 0.0) theta = sy / r;
      else          theta = 8.0 + sy / r;
    }
    else {
      r = sy;
      theta = 2.0 - sx/r;
    }
  }
  else {
    if (sx <= sy) {
      r = -sx;
      theta = 4.0 - sy / r;
    }
    else {
      r = -sy;
      theta = 6.0 + sx / r;
    }
  }
  theta *= M_PI / 4.0;

  return Vec2f{ r * std::cos(theta), r * std::sin(theta) };
}



// 該当位置の色を求める
Pixel rayTrace(const Vec3f ray_start, const Vec3f ray_vec,
               const int recursive_depth,
               const int recursive_depth_max,
               const bool back_face,
               const Model& model,
               const Bvh::BvhNode& bvh_node,
               const Hdri& bg,
               Qmc& random) {

  // BVHによるRayとMeshの交差判定
  Bvh::TestInfo test_info;
  bool has_hit = Bvh::intersect(test_info, ray_start, ray_vec, bvh_node, back_face);

  // 接触なし
  if (!has_hit) {
    // 環境マップのピクセルを使う
    Real thera = std::acos(ray_vec.y());
    Real l = std::sqrt(ray_vec.x() * ray_vec.x() + ray_vec.z() * ray_vec.z());
    Real xz = (l > 0.0) ? ray_vec.x() / l : 0.0;
    Real phi = std::acos(xz);
    if (ray_vec.z() < 0.0) {
      phi = 2.0 * M_PI - phi;
    }

    Real u = phi / (2.0 * M_PI) + 0.25;
    Real v = thera / M_PI;
    
    return bg.pixel(u, v);
  }

  const auto& material = *test_info.material;

  // 再帰上限を超えた
  // FIXME:emissiveはツールで0.0~1.0の範囲でしか設定できないので、ここで大きな値にする
  if (recursive_depth > recursive_depth_max) {
    return material.emissive() * 100;
  }

  // 鏡面反射を再帰で求める
  Pixel reflection_pixel(Pixel::Zero());
  if (!material.reflective().isZero()) {
    Vec3f reflection_vec = reflectVec(ray_vec, test_info.hit_normal);
    // TIPS:ベクトルが同じ場所に衝突しないように少し進めておく
    Vec3f reflection_start = (test_info.hit_pos + reflection_vec * 0.001);

    reflection_pixel = rayTrace(reflection_start, reflection_vec,
                                recursive_depth + 1,
                                recursive_depth_max,
                                false,
                                model,
                                bvh_node,
                                bg,
                                random);
  }

  // 屈折を再帰で求める
  Pixel refraction_pixel(Pixel::Zero());
  if (!material.transparent().isZero()) {
    Real  refractive_index     = material.ior();
    Vec3f hit_normal           = test_info.hit_normal;
    bool  refraction_back_face = true;
    Real F0;

    // レイと法線との内積 >= 0 →透過物体から出る
    if (ray_vec.dot(hit_normal) >= 0.0) {
      // 反射量
      F0 = std::pow(refractive_index - 1.0, 2.0) / std::pow(refractive_index + 1.0, 2.0);

      hit_normal = -hit_normal;
    }
    else {
      // 反射量
      F0 = std::pow(1.0 - refractive_index, 2.0) / std::pow(1.0 + refractive_index, 2.0);

      // Cheetah3Dの屈折率は素材の値なので、入射の場合、真空(1.0)との比にする
      refractive_index = 1 / refractive_index;
    }

    // 全反射??
    Real ddn = ray_vec.dot(hit_normal);
    Real cos2t = 1.0 - refractive_index * refractive_index * (1.0 - ddn * ddn);
    if (cos2t < 0.0) {
      Vec3f reflection_vec = reflectVec(ray_vec, test_info.hit_normal);

      // TIPS:ベクトルが同じ場所に衝突しないように少し進めておく
      Vec3f reflection_start = (test_info.hit_pos + reflection_vec * 0.001);
      
      refraction_pixel = rayTrace(reflection_start, reflection_vec,
                                  recursive_depth + 1,
                                  recursive_depth_max,
                                  false,
                                  model,
                                  bvh_node,
                                  bg,
                                  random);
    }
    else {
      Vec3f refraction_vec = refractVec(ray_vec, hit_normal, refractive_index);
      // TIPS:屈折ベクトルが同じ場所に衝突しないようにちょこっとだけ進めておく
      Vec3f refraction_start = (test_info.hit_pos + refraction_vec * 0.001);

      // 屈折後の光の量
      Real Re = F0 + (1.0 - F0) * std::pow(1.0 + ddn, 5.0);
      Real Tr = 1.0 - Re;
      
      refraction_pixel = rayTrace(refraction_start, refraction_vec,
                                  recursive_depth + 1,
                                  recursive_depth_max,
                                  refraction_back_face,
                                  model,
                                  bvh_node,
                                  bg,
                                  random) * Tr;
    }
  }

  // 拡散反射
  Pixel light_diffuse = Pixel::Zero();
  if (!material.diffuse().isZero()) {
    // TIPS:ベクトルが同じ場所に衝突しないように少し浮かせる
    Vec3f passtarce_start(test_info.hit_pos + test_info.hit_normal * 0.001);
    Vec3f passtarce_vec = radiationVector_qmc(test_info.hit_normal, random);

    light_diffuse = rayTrace(passtarce_start, passtarce_vec,
                             recursive_depth + 1,
                             recursive_depth_max,
                             false,
                             model,
                             bvh_node,
                             bg,
                             random);
  }
  
  Real reflect_value = 1.0 - material.reflective().maxCoeff();
  Real refract_value = 1.0 - material.transparent().maxCoeff();

  Pixel diffuse_color = material.diffuse();
#if 0
  if (material.hasTexture()) {
    Real u = test_info.hit_uv.x();
    Real v = test_info.hit_uv.y();
    
    diffuse_color = material.texture().pixel(u, v);
  }
#endif

  return diffuse_color * light_diffuse * reflect_value * refract_value
       + material.reflective() * reflection_pixel
       + material.transparent() * refraction_pixel
       + material.emissive() * 100;
}


// レンダリング用の情報
struct RenderInfo {
  Vec2i size;

  std::vector<GLint> viewport;

  Camera3D camera;
  Pixel ambient;
  std::vector<Light> lights;
  Model model;
  Bvh::BvhNode bvh_node;

  Hdri bg;

  int subpixel_num;
  int sample_num;
  int recursive_depth;

  Real focal_distance;
  Real lens_radius;
  
  Real exposure;


  RenderInfo(const int width, const int height,
             const std::vector<GLint>& src_viewport,
             const Camera3D& src_camera,
             const Pixel& src_ambient,
             const std::vector<Light>& src_lights,
             const Model& src_model,
             const std::string& bg_path,
             const int src_subpixel_num,
             const int src_sample_num,
             const int src_recursive_depth,
             const Real src_focal_distance,
             const Real src_lens_radius,
             const Real src_exposure) :
    size(width, height),
    viewport(src_viewport),
    camera(src_camera),
    ambient(src_ambient),
    lights(src_lights),
    model(src_model),
    bvh_node(Bvh::createFromModel(src_model)),
    bg(bg_path),
    subpixel_num(src_subpixel_num),
    sample_num(src_sample_num),
    recursive_depth(src_recursive_depth),
    focal_distance(src_focal_distance),
    lens_radius(src_lens_radius),
    exposure(src_exposure)
  { }
};


// 露出計算
// exposure 露出値(マイナス値)
Real expose(const Real light, const Real exposure) {
  return 1.0 - std::exp(light * exposure);
}


bool render(std::shared_ptr<std::vector<u_char> > row_image,
            std::shared_ptr<RenderInfo> info) {
  bool do_dof = info->lens_radius > 0.0;

  // 画面中心から奥に伸びるベクトル
  Vec3f to_far_z = info->camera.posToWorld(Vec3f(info->size.x() / 2, info->size.y() / 2, 1.0),
                                           Affinef::Identity(), info->viewport);
  to_far_z.normalize();
  
  for (int iy = 0; iy < info->size.y(); ++iy) {
    std::vector<Pixel> image(info->size.x());

    for (int ix = 0; ix < info->size.x(); ++ix) {
      Pixel sub_pixel = Pixel::Zero();
      
      for (int i = 0; i < info->subpixel_num; ++i) {

        for (int h = 0; h < info->sample_num; ++h) {
          // １ピクセル内で乱数が完結するよう調節
          Qmc render_random(h + i * info->sample_num + (ix + iy * info->size.x()) * (info->sample_num * info->subpixel_num));
          
          Real r1 = 2.0 * render_random.next();
          Real r2 = 2.0 * render_random.next();
        
          Real x = ix + ((r1 < 1.0) ? std::sqrt(r1) - 1.0 : 1.0 - std::sqrt(2.0 - r1));
          Real y = iy + ((r2 < 1.0) ? std::sqrt(r2) - 1.0 : 1.0 - std::sqrt(2.0 - r2));
        
          // 画面最前→最奥へ伸びる線分を計算
          Vec3f ray_start = info->camera.posToWorld(Vec3f(x, y, 0.0),
                                                    Affinef::Identity(), info->viewport);
          Vec3f ray_end = info->camera.posToWorld(Vec3f(x, y, 1.0),
                                                  Affinef::Identity(), info->viewport);

          Vec3f ray_vec = (ray_end - ray_start).normalized();
          
          if (do_dof) {
            // レンズの屈折をシミュレーション(被写界深度)
            // SOURCE:https://github.com/githole/simple-pathtracer/tree/simple-pathtracer-DOF

            // フォーカスが合う位置
            Real ft = std::abs(info->focal_distance / to_far_z.dot(ray_vec));
            Vec3f focus_pos = ray_start + ray_vec * ft;

            // 適当に決めたレンズの通過位置とフォーカスが合う位置からRayを作り直す(屈折効果)
            Vec2f lens = concentricSampleDisk(render_random.next(), render_random.next()) * info->lens_radius;
            ray_start.x() += lens.x();
            ray_start.y() += lens.y();
            ray_vec = (focus_pos - ray_start).normalized();
          }
          
          sub_pixel += rayTrace(ray_start, ray_vec,
                                0,
                                info->recursive_depth,
                                false,
                                info->model,
                                info->bvh_node,
                                info->bg,
                                render_random);
        }
      }
      image[ix] = sub_pixel / (info->sample_num * info->subpixel_num);
    }

    {
      // 1ライン毎にイメージを生成
      // 0.0~1.0のピクセルの値を0~255へ正規化
      int index = iy * info->size.x() * 3;
      Real exposure = info->exposure;
      std::for_each(image.begin(), image.end(),
                    [&row_image, &index, &exposure](const Pixel& pixel) {
                      (*row_image)[index + 0] = expose(pixel.x(), exposure) * 255;
                      (*row_image)[index + 1] = expose(pixel.y(), exposure) * 255;
                      (*row_image)[index + 2] = expose(pixel.z(), exposure) * 255;
                      index += 3;
                    });
    }
  }

  return true;
}

}

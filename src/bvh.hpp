
#pragma once

//
// BVH
// SOURCE:http://qiita.com/omochi64/items/9336f57118ba918f82ec
//

#include "defines.hpp"
#include <deque>
#include "collision.hpp"
#include "model.hpp"


namespace Bvh {

struct BBox {
  Vec3f inf;
  Vec3f sup;
};

struct BvhTriangle {
  const Triangle* triangle;
  const Triangle* normal;
  const Triangle* uv;

  const Material* material;

  BBox  bbox;
  Vec3f center;
};


struct BvhNode {
  BBox bbox;
  std::vector<BvhNode> children;

  std::deque<BvhTriangle> triangles;
};


// AABB の表面積計算
Real surfaceArea(const BBox& bbox) {
  auto d = bbox.sup - bbox.inf;
  return 2 * (d.x() * d.y() + d.x() * d.z() + d.y() * d.z());
}

// 空のAABBを生成
BBox emptyAABB() {
  BBox bbox = { { FLT_MAX, FLT_MAX, FLT_MAX }, { -FLT_MAX, -FLT_MAX, -FLT_MAX } };
  return bbox;
}

// 2つのAABBをマージ
BBox mergeAABB(const BBox& bbox1, const BBox& bbox2) {
  BBox merged_box;

  for (int i = 0; i < 3; ++i) {
    merged_box.inf(i) = std::min(bbox1.inf(i), bbox2.inf(i));
    merged_box.sup(i) = std::max(bbox1.sup(i), bbox2.sup(i));
  }

  return merged_box;
}

// ポリゴンリストからAABBを生成
BBox createAABBfromTriangles(const std::deque<BvhTriangle>& triangles) {
  auto bbox = emptyAABB();
  
  std::for_each(triangles.begin(), triangles.end(),
                [&bbox](const BvhTriangle& t) {
                  bbox = mergeAABB(t.bbox, bbox);
                });

  return bbox;
}


const Real T_tri  = 1;                              // 適当
const Real T_aabb = 1;                              // 適当


// FIXME:trianglesの内容は破壊される
BvhNode construct(std::deque<BvhTriangle>& triangles) {
  BvhNode node;

  // 全体を囲うAABBを計算
  node.bbox = createAABBfromTriangles(triangles);

  // 領域分割をせず、polygons を含む葉ノードを構築する場合を暫定の bestCost にする
  Real bestCost = T_tri * triangles.size();

  int bestAxis       = -1;                          // 分割に最も良い軸 (0:x, 1:y, 2:z)
  int bestSplitIndex = -1;                          // 最も良い分割場所
  Real SA_root = surfaceArea(node.bbox);            // ノード全体のAABBの表面積

  for (int axis = 0; axis < 3; ++axis) {
    // ポリゴンリストを、それぞれのAABBの中心座標を使い、axis でソートする
    std::sort(triangles.begin(), triangles.end(),
              [axis](const BvhTriangle& a, const BvhTriangle& b) {
                return a.center(axis) < b.center(axis);
              });
    
    std::deque<BvhTriangle> s1;
    std::deque<BvhTriangle> s2(triangles);         // 分割された2つの領域
    auto s1bbox = emptyAABB();                     // S1のAABB
    
    // AABBの表面積リスト。s1SA[i], s2SA[i] は、
    // 「S1側にi個、S2側に(polygons.size()-i)個ポリゴンがあるように分割」したときの表面積
    std::vector<Real> s1SA(triangles.size() + 1, FLT_MAX);
    std::vector<Real> s2SA(triangles.size() + 1, FLT_MAX);

    // 可能な分割方法について、s1側の AABB の表面積を計算
    for (int i = 0; i <= triangles.size(); ++i) {
      // 現在のS1のAABBの表面積を計算
      s1SA[i] = std::fabs(surfaceArea(s1bbox));
      if (s2.size() > 0) {
        // s2側で、axis について最左 (最小位置) にいるポリゴンをS1の最右 (最大位置) に移す
        BvhTriangle p = s2.front();
        s1.push_back(p);
        s2.pop_front();

        // 移したポリゴンのAABBをマージしてS1のAABBとする
        s1bbox = mergeAABB(s1bbox, p.bbox);
      }
    }

    // 逆にS2側のAABBの表面積を計算しつつ、SAH を計算
    auto s2bbox = emptyAABB();
    
    for (int i = triangles.size(); i >= 0; --i) {
      s2SA[i] = std::fabs(surfaceArea(s2bbox));     // 現在のS2のAABBの表面積を計算
      
      if (s1.size() > 0 && s2.size() > 0) {
        // SAH-based cost の計算
        Real cost = 2 * T_aabb + (s1SA[i] * s1.size() + s2SA[i] * s2.size()) * T_tri / SA_root;

        // 最良コストが更新されたか？
        if (cost < bestCost) {
          bestCost       = cost;
          bestAxis       = axis;
          bestSplitIndex = i;
        }
      }

      if (s1.size() > 0) {
        // S1側で、axis について最右にいるポリゴンをS2の最左に移す
        auto p = s1.back();
        s2.push_front(p);
        s1.pop_back();
        
        // 移したポリゴンのAABBをマージしてS2のAABBとする
        s2bbox = mergeAABB(s2bbox, p.bbox);
      }
    }
  }

  if (bestAxis == -1) {
    // 現在のノードを葉ノードとするのが最も効率が良い結果になった
    // => 葉ノードの作成
    node.triangles = triangles;
  }
  else {
    // bestAxis に基づき、左右に分割
    // bestAxis でソート
    std::sort(triangles.begin(), triangles.end(),
         [bestAxis](const BvhTriangle& a, const BvhTriangle& b) {
                return a.center(bestAxis) < b.center(bestAxis);
         });

    // ポリゴンリストを分割
    std::deque<BvhTriangle> left(triangles.begin(), triangles.begin() + bestSplitIndex);
    std::deque<BvhTriangle> right(triangles.begin() + bestSplitIndex, triangles.end());

    // 再帰処理
    node.children.resize(2);
    node.children[0] = construct(left);
    node.children[1] = construct(right);
  }

  return node;
}

// ModelからBVHを生成
BvhNode createFromModel(const Model& model) {
  std::deque<BvhTriangle> triangles;

  const auto& mesh     = model.mesh();
  const auto& material = model.material();

  int polygon_num = 0;
  
  for (const auto& m : mesh) {
    const auto& polygons = m->polygons();
    const auto& normals  = m->normals();
    const auto& uvs      = m->uvs();
    const auto& mat      = material[m->materialIndex()];
    bool has_texture = mat.hasTexture();

    polygon_num += polygons.size();
    
    for (size_t ip = 0; ip < polygons.size(); ++ip) {
      BvhTriangle t;

      t.triangle = &polygons[ip];
      t.normal   = &normals[ip];
      t.uv       = has_texture ? &uvs[ip] : nullptr;
      t.material = &mat;

      for (int i = 0; i < 3; ++i) {
        t.bbox.inf(i) = std::min({ polygons[ip].a(i), polygons[ip].b(i), polygons[ip].c(i) });
        t.bbox.sup(i) = std::max({ polygons[ip].a(i), polygons[ip].b(i), polygons[ip].c(i) });

        t.center(i) = (t.bbox.inf(i) + t.bbox.sup(i)) / 2.0;
      }

      triangles.push_back(t);
    }
  }

  DOUT << "polygon:" << polygon_num << std::endl;
  
  return construct(triangles);
}


// AABBと光線(p + td)との交差判定と交差点
bool testRayAABB(const Vec3f& p, const Vec3f& d, const BBox& b) {
	Real tmin = 0.0;
	Real tmax = FLT_MAX;

	for (u_int i = 0; i < 3; ++i) {
		if (std::abs(d(i)) < FLT_EPSILON) {
			if (p(i) < b.inf(i) || p(i) > b.sup(i)) return false;
		}
		else {
			Real ood = 1.0 / d(i);
			Real t1 = (b.inf(i) - p(i)) * ood;
			Real t2 = (b.sup(i) - p(i)) * ood;

			if (t1 > t2) std::swap(t1, t2);
      tmin = std::max(t1, tmin);
      tmax = std::min(t2, tmax);

			// if (t1 > tmin) tmin = t1;
			// if (t2 < tmax) tmax = t2;

			if (tmin > tmax) return false;
		}
	}

	return true;
}


struct TestInfo {
  Real distance;

  Vec3f hit_pos;
  Vec3f hit_normal;
  Vec3f hit_uv;

  const Material* material;

  TestInfo() :
    distance(FLT_MAX)
  {}
};


bool intersect(TestInfo& res, const Vec3f& ray_start, const Vec3f& ray_vec, const BvhNode& node, const bool back_face) {
  bool  hit_res = false;

  if (!testRayAABB(ray_start, ray_vec, node.bbox)) return false;

  if (node.children.empty()) {
    // AABB内のポリゴンとの交差判定
    for (const auto& t : node.triangles) {
      Vec3f hit_pos;
      Real  hit_t;
      Vec3f hit_normal;
      Vec3f hit_center;

      if (testRayTriangle(hit_pos, hit_t, hit_normal, hit_center,
                          ray_start, ray_vec, *t.triangle, back_face)) {
        if (hit_t < res.distance) {
          hit_res = true;

          res.distance = hit_t;
          res.hit_pos  = hit_pos;
          res.material = t.material;

          res.hit_normal = (t.normal->a * hit_center.x()
                          + t.normal->b * hit_center.y()
                          + t.normal->c * hit_center.z()).normalized();

          if (t.uv) {
            res.hit_uv = t.uv->a * hit_center.x()
                       + t.uv->b * hit_center.y()
                       + t.uv->c * hit_center.z();
          }
        }
      }
    }
  }
  else {
    if (intersect(res, ray_start, ray_vec, node.children[0], back_face)) hit_res = true;
    if (intersect(res, ray_start, ray_vec, node.children[1], back_face)) hit_res = true;
  }
  
  return hit_res;
}

}

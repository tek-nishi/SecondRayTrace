
#pragma once

//
// Open Asset Importer からシーンを読み込む
//

#include "defines.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <utility>
#include <vector>
#include "camera3D.hpp"
#include "light.hpp"
#include "model.hpp"


namespace {

struct Scene {
  Camera3D           camera;
  Pixel              ambient;
  std::vector<Light> lights;
  Model              model;
};

}


namespace SceneLoader {

// 読み込みフラグ
// TIPS:データ内の階層構造を計算済みの状態にする
enum {
  import_flags = aiProcess_JoinIdenticalVertices |
                 aiProcess_Triangulate |
                 aiProcess_FlipUVs |
                 aiProcess_SortByPType |
                 aiProcess_OptimizeMeshes |
                 aiProcess_PreTransformVertices
};


Real horizontalFov(const Real fovx, const Real near_z, const Real aspect) {
  // fovyとnear_zから投影面の幅の半分を求める
  Real half_w = std::tan(fovx / 2) * near_z;

  // 表示画面の縦横比から、投影面の高さの半分を求める
  Real half_h = half_w / aspect;

  // 投影面の高さ半分とnear_zから、縦画面の時のfovyが求まる
  return std::atan(half_h / near_z) * 2;
}


Scene load(const std::string& path) {
  Assimp::Importer importer;
  const auto* ai_scene = importer.ReadFile(path, import_flags);
  if (!ai_scene) {
    DOUT << importer.GetErrorString() << std::endl;
    throw;
  }

  // カメラ
  if (!ai_scene->HasCameras()) {
    throw "No camera.";
  }

  // 最初のカメラが対象
  aiCamera* scene_camera = *ai_scene->mCameras;

  // assimpのfovxからfovyへ変換
  Real fovy = horizontalFov(scene_camera->mHorizontalFOV,
                            scene_camera->mClipPlaneNear,
                            scene_camera->mAspect);

  // 読み込みデータを構築
  Scene scene{
    { fovy,
      scene_camera->mClipPlaneNear, scene_camera->mClipPlaneFar },
    {},
    {},
    { ai_scene, path },
  };

  // カメラの位置と向きは逆向きに設定
  const aiVector3D& pos = scene_camera->mPosition;
  scene.camera.eyePosition({ -pos.x, -pos.y, -pos.z });

  // Matrix→Quaternion
  aiMatrix4x4 mat4x4;
  scene_camera->GetCameraMatrix(mat4x4);
  aiQuaternion rotate = aiQuaternion(aiMatrix3x3{ mat4x4 });

  // FIXME:カメラの向きを逆にするのに、Y軸に180度回転している
  Quatf r{ rotate.w, rotate.x, rotate.y, rotate.z };
  scene.camera.rotate(Quatf{ AngleAxis(M_PI, Vec3f::UnitY()) } * r);

  
  // ライト
  if (!ai_scene->HasLights()) {
    scene.ambient = Pixel::Zero();
  }

  // Cheetah3Dの光源はパラメーターから種類が判別できないので
  // １個目を環境光と決め打ち
  for (int i = 0; i < ai_scene->mNumLights; ++i) {
    const auto* light = ai_scene->mLights[i];

    switch (i) {
    case 0:
      {
        const auto& ambiemt = light->mColorAmbient;
        scene.ambient << ambiemt.r, ambiemt.g, ambiemt.b;
      }
      break;

    default:
      {
        const auto& diffuse = light->mColorDiffuse;
        const auto& pos     = light->mPosition;

        Light light = {
          Pixel(diffuse.r, diffuse.g, diffuse.b),
          Vec3f(pos.x, pos.y, pos.z),
        };
        scene.lights.push_back(light);
      }
      break;
    }
  }
  
  return scene;
}

}

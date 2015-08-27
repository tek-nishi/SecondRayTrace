
#pragma once

//
// Open Asset Importer テスト
//

#include "defines.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "fileUtil.hpp"
#include "texMng.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "node.hpp"


// リンクするライブラリの定義(Windows)
#if defined (_MSC_VER)
#ifdef DEBUG
#pragma comment (lib, "assimpd.lib")
#pragma comment (lib, "zlibd.lib")
#else
#pragma comment (lib, "assimp.lib")
#pragma comment (lib, "zlib.lib")
#endif
#endif


namespace {

class Model {
  TexMng textures_;

  std::vector<std::shared_ptr<Mesh> > meshes_;
  std::vector<Material> material_;
  Node root_node_;

  // 読み込みフラグ
  enum {
    import_flags = aiProcess_JoinIdenticalVertices |
                   aiProcess_Triangulate |
                   aiProcess_FlipUVs |
                   aiProcess_SortByPType |
                   aiProcess_OptimizeMeshes |
                   aiProcess_OptimizeGraph
    //aiProcess_PreTransformVertices
  };

  
public:
  Model(const std::string& path) {
    // Open Asset Importerを利用してモデルデータを読み込む
    Assimp::Importer importer;
    const auto* scene = importer.ReadFile(path, import_flags);
    if (!scene) {
      DOUT << importer.GetErrorString() << std::endl;
    }
    DOUT << "Mesh:" << scene->mNumMeshes << std::endl;
    DOUT << "Material:" << scene->mNumMaterials << std::endl;

    Model::Model(scene, path);
  }

  Model(const aiScene* scene, const std::string& path) {
    DOUT << "Model()" << std::endl;

    // メッシュ生成
    for (u_int i = 0; i < scene->mNumMeshes; ++i) {
      const auto& scene_mesh = *(scene->mMeshes[i]);
      // TIPS:コンテナ内に直接Meshを生成する
      meshes_.emplace_back(std::make_shared<Mesh>(scene_mesh));
    }

    // マテリアル
    for (u_int i = 0; i < scene->mNumMaterials; ++i) {
      const auto& scene_material = *(scene->mMaterials[i]);
      material_.emplace_back(scene_material, textures_, getDirectoryname(path));
    }

    // 階層構造を生成
    root_node_.setup(scene->mRootNode);
  }

  ~Model() {
    DOUT << "~Model()" << std::endl;
  }


  const std::vector<std::shared_ptr<Mesh> >& mesh() const { return meshes_; }
  const std::vector<Material>& material() const {return material_; }

  std::vector<Material>& material() {return material_; }

  
  const Node& rootNode() const { return root_node_; }
  Node& rootNode() { return root_node_; }


private:

  
};

}


#pragma once

//
// モデルの階層構造を定義
// 行列、子供の階層、含んでいるメッシュ
// TODO:計算済みのカレント行列を用意
//

#include "defines.hpp"
#include <deque>
#include <vector>
#include <assimp/scene.h>
#include "vector.hpp"
#include "matrix.hpp"


namespace {

class Node {
  Affinef matrix_;

#if defined (_MSC_VER)
  using Container = std::vector<Node, Eigen::aligned_allocator<Node> >;
  // FIXME:16bytes alignmentしないとWindowsでエラーになる
#else
  using Container = std::vector<Node>;
#endif

  Container child_nodes_;

  std::deque<u_int> mesh_indexes_;
  std::string name_;

  
public:
  Node() {}
  
  explicit Node(const aiNode* node) :
    name_(node->mName.C_Str())
  {
    DOUT << "Node()" << std::endl;
    create(node);
  }

  ~Node() {
    DOUT << "~Node()" << std::endl;
  }

  
  void setup(const aiNode* node) {
    child_nodes_.clear();
    mesh_indexes_.clear();
    create(node);
  }

  const std::string& name() const { return name_; }
  
  const Affinef& matrix() const { return matrix_; }

  const Container& childs() const { return child_nodes_; }
  Container& childs() { return child_nodes_; }

  const std::deque<u_int>& meshIndexes() const { return  mesh_indexes_; }

  // 自分を含む、子供の階層数を返す
  u_int numNode() const {
    u_int num = 1;
    for (const auto& node : child_nodes_) {
      num += node.numNode();
    }
    return num;
  }


private:
  void create(const aiNode* node) {
    // 行列のコピー
    for (u_int low = 0; low < 4; ++low) {
      for (u_int colm = 0; colm < 4; ++colm) {
        matrix_(colm, low) = node->mTransformation[colm][low];
      }
    }

    DOUT << "Node name:" << std::string(node->mName.C_Str()) << std::endl;

    // メッシュ配列
    for(u_int i = 0; i < node->mNumMeshes; ++i) {
      mesh_indexes_.push_back(node->mMeshes[i]);
    }

    // 子ノード生成
    // あらかじめvectorのサイズを予約し、コピーが発生するのを防ぐ
    child_nodes_.reserve(node->mNumChildren);
    for (u_int i = 0; i < node->mNumChildren; ++i) {
      // メッシュも子供も存在しないノードはスキップ
      // ※カメラとかライトが該当する
      if (!node->mChildren[i]->mNumMeshes && !node->mChildren[i]->mNumChildren) continue;

      child_nodes_.emplace_back(node->mChildren[i]);
      // TIPS:vector内に直接Nodeを生成
    }
  }
  
};

}

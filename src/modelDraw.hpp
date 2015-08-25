
#pragma once

//
// モデル描画
//

#include "defines.hpp"
#include "model.hpp"


namespace {

// マテリアルの設定
void setupMaterial(const Material& material) {
  // テクスチャを割り当てる
  if (material.hasTexture()) material.bindTexture();

  // マテリアルの色を設定
  const Vec3f& diffuse = material.diffuse();
  GLfloat mat_diffuse[] = { GLfloat(diffuse.x()), GLfloat(diffuse.y()), GLfloat(diffuse.z()), 1.0f };
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);

  // スペキュラーを設定
  const Vec3f& specular = material.specular();
  GLfloat mat_spelular[] = { GLfloat(specular.x()), GLfloat(specular.y()), GLfloat(specular.z()), 1.0f };
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spelular);

  glMaterialf(GL_FRONT, GL_SHININESS, material.shininess());

  // エミッシブ(自己発光)
  const Vec3f& emissive = material.emissive();
  GLfloat mat_emissive[] = { GLfloat(emissive.x()), GLfloat(emissive.y()), GLfloat(emissive.z()), 1.0f };
  glMaterialfv(GL_FRONT, GL_EMISSION, mat_emissive);
  
  // テクスチャにスペキュラーを適用する時は、色を分離して計算する
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
}

// メッシュの描画
void meshDraw(const Mesh& mesh, const bool use_texture) {
  mesh.bindArrayBuffer();
  mesh.bindElementBuffer();

  // 頂点
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(Mesh::Body), 0);

  // 法線
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(Mesh::Body), static_cast<GLbyte*>(0) + offsetof(Mesh::Body, normal));

  // テクスチャ座標
  if (use_texture) {
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Mesh::Body), static_cast<GLbyte*>(0) + offsetof(Mesh::Body, uv));
  }

  // 面ごとの頂点インデックス配列を使って描画
  glDrawElements(GL_TRIANGLES, mesh.points(), GL_UNSIGNED_INT, 0);

  // 頂点バッファへの関連付けを解除
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  if (use_texture) {
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  mesh.unbindElementBuffer();
  mesh.unbindArrayBuffer();
}

// １階層の描画
void nodeDraw(const Node& node,
              const std::vector<std::shared_ptr<Mesh> >& mesh,
              const std::vector<Material>& material) {
  glPushMatrix();
  glMultMatrix(node.matrix().data());
  
  for (const u_int mesh_index : node.meshIndexes()) {
    const auto& l_mesh = *mesh[mesh_index];
    const auto& l_material = material[l_mesh.materialIndex()];

    setupMaterial(l_material);
    meshDraw(l_mesh, l_material.hasTexture());
  }
  
  for (auto& child : node.childs()) {
    nodeDraw(child, mesh, material);
  }
  glPopMatrix();
}

void modelDraw(const Model& model) {
  nodeDraw(model.rootNode(), model.mesh(), model.material());
}

}


#pragma once

//
// モデルのメッシュを定義
// ※頂点、法線、テクスチャ座標
// FIXME:OpenGL依存
//

#include "defines.hpp"
#include <assimp/scene.h>
#include <cfloat>
#include <boost/noncopyable.hpp>
#include "vector.hpp"
#include "glBuffer.hpp"
#include "collision.hpp"


namespace {

class Mesh : private boost::noncopyable {
public:
	struct Vtx {
		GLfloat x, y, z;
	};
  
	struct Uv {
		GLfloat u, v;
	};
  
	struct Body {
		Vtx vertex;
		Vtx normal;
		Uv uv;
	};
  
	struct Face {
		GLuint v1, v2, v3;
	};

  
private:
  bool has_normal_;
  bool has_texture_;
  u_int faces_;
	GLuint points_;
  u_int material_index_;

  Vec3f min_pos_;
  Vec3f max_pos_;

  GlBuffer body_;
  GlBuffer face_;

  std::vector<Triangle> polygons_;
  std::vector<Triangle> normals_;
  std::vector<Triangle> uvs_;

  AABBVolume bbox_;

  
public:
  explicit Mesh(const aiMesh& mesh) :
    has_normal_(mesh.HasNormals()),
    has_texture_(mesh.HasTextureCoords(0)),
    faces_(mesh.mNumFaces),
    points_(mesh.mNumFaces * 3),
    material_index_(mesh.mMaterialIndex),
    min_pos_(FLT_MAX, FLT_MAX, FLT_MAX),
    max_pos_(-FLT_MAX, -FLT_MAX, -FLT_MAX)
  {
    DOUT << "Mesh()" << std::endl;
#if 0
    DOUT << "Mesh Vertices:" << mesh.mNumVertices << std::endl;
    DOUT << "Mesh Normals:" << mesh.HasNormals() << std::endl;
    DOUT << "Mesh UV:" << mesh.HasTextureCoords(0) << std::endl;
    DOUT << "Mesh VtertexColors:" << mesh.HasVertexColors(0) << std::endl;
    DOUT << "Mesh Faces:" << mesh.mNumFaces << std::endl;
    DOUT << "Mesh Material:" << mesh.mMaterialIndex << std::endl;
#endif
    
    // 頂点情報を生成
    const auto* v  = mesh.mVertices;
    const auto* n  = mesh.mNormals;
    const auto* uv = mesh.mTextureCoords[0];

    std::vector<Body> body;
    body.reserve(mesh.mNumVertices);
    // TIPS:あらかじめvectorのサイズを予約し、push_backによるコピーを防ぐ

    for (u_int i = 0; i < mesh.mNumVertices; ++i) {
      Body obj;
      obj.vertex.x = v->x;
      obj.vertex.y = v->y;
      obj.vertex.z = v->z;
      if (has_normal_) {
        obj.normal.x = n->x;
        obj.normal.y = n->y;
        obj.normal.z = n->z;
      }
      if (has_texture_) {
        obj.uv.u = uv->x;
        obj.uv.v = uv->y;
      }
      body.push_back(obj);

      min_pos_.x() = std::min(min_pos_.x(), Real(v->x));
      min_pos_.y() = std::min(min_pos_.y(), Real(v->y));
      min_pos_.z() = std::min(min_pos_.z(), Real(v->z));

      max_pos_.x() = std::max(max_pos_.x(), Real(v->x));
      max_pos_.y() = std::max(max_pos_.y(), Real(v->y));
      max_pos_.z() = std::max(max_pos_.z(), Real(v->z));
      
      ++v;
      ++n;
      ++uv;
    }

    // 面情報を生成
    auto* f = mesh.mFaces;
    std::vector<Face> face;
    face.reserve(mesh.mNumFaces);

    for (u_int i = 0; i < mesh.mNumFaces; ++i) {
      // 三角ポリゴン以外はエラー
      assert(f->mNumIndices == 3);

      Face obj;
      obj.v1 = f->mIndices[0];
      obj.v2 = f->mIndices[1];
      obj.v3 = f->mIndices[2];
      face.push_back(obj);

      ++f;
    }

    // OpenGLのFrame Buffer Objectを生成して、頂点データを転送する
    body_.setData(GL_ARRAY_BUFFER, body);
    face_.setData(GL_ELEMENT_ARRAY_BUFFER, face);


    // 三角形ポリゴンを生成
    polygons_.reserve(mesh.mNumFaces);
    normals_.reserve(mesh.mNumFaces);
    if (has_texture_) uvs_.reserve(mesh.mNumFaces);

    for (u_int i = 0; i < mesh.mNumFaces; ++i) {
      const auto& v1 = body[face[i].v1];
      const auto& v2 = body[face[i].v2];
      const auto& v3 = body[face[i].v3];

      // Triangle polygon = makeTriangle(Vec3f(v1.vertex.x, v1.vertex.y, v1.vertex.z),
      //                                 Vec3f(v2.vertex.x, v2.vertex.y, v2.vertex.z),
      //                                 Vec3f(v3.vertex.x, v3.vertex.y, v3.vertex.z));
      
      Triangle polygon{
        { v1.vertex.x, v1.vertex.y, v1.vertex.z },
        { v2.vertex.x, v2.vertex.y, v2.vertex.z },
        { v3.vertex.x, v3.vertex.y, v3.vertex.z },
      };

      Triangle normal{
        { v1.normal.x, v1.normal.y, v1.normal.z },
        { v2.normal.x, v2.normal.y, v2.normal.z },
        { v3.normal.x, v3.normal.y, v3.normal.z },
      };
      
      polygons_.push_back(polygon);
      normals_.push_back(normal);

      if (has_texture_) {
        Triangle uv{
          { v1.uv.u, v1.uv.v, 0.0 },
          { v2.uv.u, v2.uv.v, 0.0 },
          { v3.uv.u, v3.uv.v, 0.0 },
        };
        uvs_.push_back(uv);
      }
    }

    bbox_.point  = (max_pos_ + min_pos_) / 2;
    bbox_.radius = (max_pos_ - min_pos_) / 2;
  }

  ~Mesh() {
    DOUT << "~Mesh()" << std::endl;
  }


  u_int materialIndex() const { return material_index_; }

	GLuint points() const { return points_; }
  u_int faces() const { return faces_; }

  const Vec3f& minPos() const { return min_pos_; }
  const Vec3f& maxPos() const { return max_pos_; }

  
  void bindArrayBuffer() const {
    body_.bind();
  }
  
  void unbindArrayBuffer() const {
    body_.unbind();
  }
  
  void bindElementBuffer() const {
    face_.bind();
  }
  
  void unbindElementBuffer() const {
    face_.unbind();
  }

  const std::vector<Triangle>& polygons() const { return polygons_; }
  const std::vector<Triangle>& normals() const { return normals_; }
  const std::vector<Triangle>& uvs() const { return uvs_; }

  const AABBVolume& bbox() const { return bbox_; }
  
};

}


#pragma once

//
// マテリアル管理
//

#include "defines.hpp"
#include "texMng.hpp"
#include "fileUtil.hpp"
#include "color.hpp"


namespace {

class Material {
	Pixel diffuse_;
	Pixel specular_;
  Real shininess_;
	Pixel emissive_;

  Pixel reflective_;

  Pixel transparent_;
  Real ior_;

  bool has_texture_;
  TexMng::TexPtr texture_;

  
public:
  Material(const aiMaterial& material, TexMng& tex_mng, const std::string& path) :
    diffuse_(Pixel::Zero()),
    specular_(Pixel::Zero()),
    shininess_(80.0),
    emissive_(Pixel::Zero()),
    reflective_(Pixel::Zero()),
    transparent_(Pixel::Zero()),
    ior_(1.0),
    has_texture_(false)
  {
    DOUT << "Material()" << std::endl;

    {
      // 拡散光
      aiColor3D color(0.0f, 0.0f, 0.0f);
      if (material.Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        diffuse_ << color.r, color.g, color.b;
        DOUT << "diffuse:" << diffuse_ << std::endl;
      }
    }
    
    {
      // スペキュラー
      aiColor3D color(0.0f, 0.0f, 0.0f);
      if (material.Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        specular_ << color.r, color.g, color.b;
        DOUT << "specular:" << specular_ << std::endl;
      }
    }
    
    {
      // スペキュラー拡散率
      float shininess = 0.0;
      if (material.Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        shininess_ = shininess;
        DOUT << "shininess:" << shininess_ << std::endl;
      }
    }

    {
      // エミッシブ(自己発光)
      aiColor3D color(0.0f, 0.0f, 0.0f);
      if (material.Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
        emissive_ << color.r, color.g, color.b;
        
        DOUT << "emissive:" << emissive_ << std::endl;
      }
    }

    {
      // 鏡面反射
      aiColor3D color(0.0f, 0.0f, 0.0f);
      if (material.Get(AI_MATKEY_COLOR_REFLECTIVE, color) == AI_SUCCESS) {
        reflective_ << color.r, color.g, color.b;
        
        DOUT << "reflective:" << reflective_ << std::endl;
      }
      
    }

    {
      // 屈折
      aiColor3D color(0.0f, 0.0f, 0.0f);
      if (material.Get(AI_MATKEY_COLOR_TRANSPARENT, color) == AI_SUCCESS) {
        transparent_ << color.r, color.g, color.b;
        
        DOUT << "transparent:" << transparent_ << std::endl;
      }
    }

    {
      // 屈折率
      float index_of_refraction = 1.0;
      if (material.Get(AI_MATKEY_REFRACTI, index_of_refraction) == AI_SUCCESS) {
        ior_ = index_of_refraction;

        DOUT << "IOR:" << ior_ << std::endl;
      }
    }

    {
      // テクスチャ
      aiString name;
      if (material.Get(AI_MATKEY_TEXTURE_DIFFUSE(0), name) == AI_SUCCESS) {
        // パスを除いた名前を生成
        std::string texture_file = getFilename(std::string(name.C_Str()));
        texture_ = tex_mng.read(path + "/" + texture_file);
        has_texture_ = true;
      }
    }
  }
  
  ~Material() {
    DOUT << "~Material()" << std::endl;
  }

  
  const Pixel& diffuse() const { return diffuse_; }
	const Pixel& specular() const { return specular_; }
  Real shininess() const { return shininess_; }
  const Pixel& emissive() const { return emissive_; }

  bool hasTexture() const { return has_texture_; }
  const Texture& texture() const { return *texture_.get(); };
  void bindTexture() const { texture_->bind(); }

  const Pixel& reflective() const { return reflective_; }

  const Pixel& transparent() const { return transparent_; }
  Real ior() const { return ior_; }

#ifdef DEBUG
  void ior(const Real value) { ior_ = value; } 
#endif
  
};

}

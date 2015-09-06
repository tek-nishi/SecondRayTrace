
#pragma once

//
// テクスチャ管理
//

#include "defines.hpp"
#include <string>
#include <boost/noncopyable.hpp>
#include "png.hpp"
#include "color.hpp"
#include "utils.hpp"


namespace {

class Texture : private boost::noncopyable {
	GLuint id_;

  int width_;
  int height_;

  std::vector<Pixel> pixel_;
  
	
public:
	Texture(const std::string& filename) {
    DOUT << "Texture()" << std::endl;
		glGenTextures(1, &id_);
    setupPng(filename);
	}
	
	~Texture() {
    DOUT << "~Texture()" << std::endl;
		glDeleteTextures(1, &id_);
	}


  // サイズを返す
  int width() const { return width_; }
  int height() const { return height_; }


  Pixel pixel(const Real u, const Real v) const {
    int x = int(width_ * u) % width_;
    int y = int(height_ * v) % height_;
    
    return pixel_[y * width_ + x];
  }

  
  // OpenGLのコンテキストに拘束する
	void bind() const {
		glBindTexture(GL_TEXTURE_2D, id_);
	}

  // 拘束を解除
	void unbind() const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}


private:
  // テクスチャの基本的なパラメーター設定を行う
  static void setupTextureParam() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }

  
	void setupPng(const std::string& filename) {
		Png png_obj(filename);
    width_ = png_obj.width();
    height_ = png_obj.height();
    if ((width_ != int2pow(width_)) || (height_ != int2pow(height_))) {
      DOUT << "Texture size error " << width_ << ":" << height_ << std::endl;
      // サイズが2のべき乗でなければエラー
      return;
    }

		glBindTexture(GL_TEXTURE_2D, id_);
		setupTextureParam();

		GLint type = (png_obj.type() == PNG_COLOR_TYPE_RGB) ? GL_RGB : GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, type, width_, height_, 0, type, GL_UNSIGNED_BYTE, png_obj.image());
		
    DOUT << "Texture:" << ((type == PNG_COLOR_TYPE_RGB) ? " RGB" : " RGBA") << std::endl;

    // レイトレ用にイメージを取り出す
    pixel_.reserve(width_ * height_);
    const u_char* image = png_obj.image();
    int next_pixel = (type == GL_RGB) ? 3 : 4;
    for (int i = 0; i < (width_ * height_); ++i) {
      Real r = image[0] / 255.0;
      Real g = image[1] / 255.0;
      Real b = image[2] / 255.0;
      
      pixel_.push_back({ r, g, b });
      image += next_pixel;
    }
	}
  
};

}

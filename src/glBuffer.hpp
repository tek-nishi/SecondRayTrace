
#pragma once

//
// OpenGL頂点バッファ
//

#include "defines.hpp"
#include <boost/noncopyable.hpp>


namespace {

class GlBuffer : private boost::noncopyable {
  GLuint vbo_;
  GLenum target_;

  
public:
  GlBuffer() {
    DOUT << "GlBuffer()" << std::endl;
		glGenBuffers(1, &vbo_);
  }

  ~GlBuffer() {
    DOUT << "~GlBuffer()" << std::endl;
		glDeleteBuffers(1, &vbo_);
  }


  template <typename T>
  void setData(const GLenum target, const std::vector<T>& body) {
    target_ = target;

    bind();
		glBufferData(target, sizeof(T) * body.size(), &body[0], GL_STATIC_DRAW);
    unbind();
  }


  void bind() const {
		glBindBuffer(target_, vbo_);
  }

  void unbind() const {
		glBindBuffer(target_, 0);
  }
  
};

}

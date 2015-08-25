
#pragma once

//
// グラフィック関連
//

#include "defines.hpp"
#include <GLFW/glfw3.h>


namespace {


// TIPS:Arrayだと、vec + vec や vec + scalar ができる
using Pixel = Eigen::Array<Real, 3, 1>;


class Color {
  float red_;
  float green_;
  float blue_;
  float alpha_;

  
public:  
  Color() :
    red_(0.0),
    green_(0.0),
    blue_(0.0),
    alpha_(1.0)
  {}

  Color(const float red, const float green, const float blue,
        const float alpha = 1.0) :
    red_(red),
    green_(green),
    blue_(blue),
    alpha_(alpha)
  {}

  
  // 色の変更
  float& red() { return red_; }
  float& green() { return green_; }
  float& blue() { return blue_; }
  float& alpha() { return alpha_; }


  // OpenGLへ描画色を指定
  void setToGl() const {
    glColor4f(red_, green_, blue_, alpha_);
  }
  
};

}

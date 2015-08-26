
#pragma once

//
// HDRI読み込み
// SOURCE:http://www.graphics.cornell.edu/online/formats/rgbe/
//

#include "defines.hpp"
#include <string>
#include <boost/noncopyable.hpp>
#include "color.hpp"
#include "utils.hpp"
#include "rgbe.h"


namespace {

class Hdri {
  int width_;
  int height_;
  
  std::vector<Pixel> pixel_;


public:
  Hdri(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");

    RGBE_ReadHeader(f, &width_, &height_, NULL);

    std::vector<float> image(3 * width_ * height_);
    RGBE_ReadPixels_RLE(f, &image[0], width_, height_);

    fclose(f);

    pixel_.reserve(width_ * height_);

    int index = 0;
    for (int i = 0; i < (width_ * height_); ++i) {
      Real r = image[index + 0];
      Real g = image[index + 1];
      Real b = image[index + 2];

      pixel_.push_back({ r, g, b });
      
      index += 3;
    }

    DOUT << "HDRI:" << width_ << "x" << height_ << std::endl;
  }
  

  // サイズを返す
  int width() const { return width_; }
  int height() const { return height_; }


  Pixel pixel(const Real u, const Real v) const {
    int x = int(width_ * u) % width_;
    int y = int(height_ * v) % height_;
    
    return pixel_[y * width_ + x];
  }
  
};

}

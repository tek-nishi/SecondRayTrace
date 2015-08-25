
#pragma once

//
// 乱数
//

#include "defines.hpp"
#include <random>


namespace {

class Random {
  std::mt19937 engine_;
  std::uniform_real_distribution<Real> dist_zero_to_one_;

  
public:
  Random() :
    engine_(std::mt19937::default_seed),
    dist_zero_to_one_(0.0, 1.0)
  {}


  void setSeed(const u_int new_seed) {
    engine_.seed(new_seed);
  }
  
  // [0, last) を返す
  int fromZeroToLast(const int last) {
    return engine_() % last;
  }

  // [first, last] を返す
  int fromFirstToLast(const int first, const int last) {
    return first + fromZeroToLast(last - first + 1);
  }

  
  // [0.0, 1.0] を返す
  Real fromZeroToOne() {
    return dist_zero_to_one_(engine_);
  }

  // [first, last] を返す
  Real fromFirstToLast(const Real first, const Real last) {
    return first + (last - first) * fromZeroToOne();
  }
  
};

}

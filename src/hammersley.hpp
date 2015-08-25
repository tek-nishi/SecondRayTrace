
#pragma once

#include "defines.hpp"
#include <cmath>


//
// Hammersley 点群生成
//

namespace {

class Hammersley {
  int n_;
  int dim_;
  std::vector<Real> vector_;

  int current_index_;

  
public:
  Hammersley(const int n, const int dim) :
    n_(n),
    dim_(dim),
    vector_(create(n, dim)),
    current_index_(0)
  { }

  
  Real operator()() {
    Real value = vector_[current_index_];
    current_index_ = (current_index_ + 1) % (n_ * dim_);

    return value;
  }


  void setIndex(const int index) {
    current_index_ = index * dim_;
  }

  
private:
  
  static Real vdC(int i, const int base) {
    Real factor = 1.0 / base;
    Real f = factor;
    Real h = 0.0;

    while (i > 0) {
      h += (i % base) * factor;
      i /= base;
      factor *= f;
    }
    
    return h;
  }

  // n   数
  // dim 次元
  static std::vector<Real> create(const int n, const int dim) {
    const int primes[10] = { 2, 3, 5, 7, 9, 11, 13, 17, 19, 23 };
    int i;
    int j;

    assert(dim < 10);

    std::vector<Real> x(n * dim);
    for (j = 0; j < n; ++j) {
      x[j * dim + 0] = Real(j) / Real(n);
      for (i = 1; i < dim; ++i) {
        x[j * dim + i] = vdC(j, primes[i - 1]);
      }
    }

    return x;
  }
  
};

}

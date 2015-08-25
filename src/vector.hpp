
#pragma once

//
// ベクトル定義
//

#include "defines.hpp"
#include <Eigen/Geometry>


namespace {


using Vec2i = Eigen::Matrix<int, 2, 1>;
using Vec2f = Eigen::Matrix<Real, 2, 1>;
using Vec3f = Eigen::Matrix<Real, 3, 1>;
using Vec4f = Eigen::Matrix<Real, 4, 1>;


using Quatf = Eigen::Quaternion<Real>;
using AngleAxis = Eigen::AngleAxis<Real>;


// 反射(v:ベクトル n: 法線 r:反射係数 2.0で鏡面反射 1.0で反射なし)
template<typename T>
T reflectVec(const T& v, const T& n, const Real r = 2.0) {
  return v - r * n.dot(v) * n;
}

// 屈折
// ior = 屈折率の比
template<typename T>
T refractVec(const T& v, const T& n, const Real ior = 1.0) {
  Real k = 1.0 - ior * ior * (1.0 - n.dot(v) * n.dot(v));
  if (k < 0.0) return T::Zero();
  return ior * v - (ior * n.dot(v) + std::sqrt(k)) * n;
}

}


#pragma once

//
// 行列定義
//

#include "defines.hpp"
#include <utility>
#include <vector>
#include <Eigen/Geometry>
#include "vector.hpp"


namespace {

typedef Eigen::Translation<Real, 3> Translation;
typedef Eigen::Transform<Real, 3, Eigen::Affine> Affinef;

// FIXME:コンストラクタでx,y,z全て渡さないと初期値が未定義になる
typedef Eigen::DiagonalMatrix<Real, 3> Scaling; 

typedef Eigen::Matrix<Real, 4, 4> Mat3f;
typedef Eigen::Matrix<Real, 4, 4> Mat4f;



//
// ３軸回転からクオータニオンを生成
//
Quatf rotateXYZ(const Real rad_x, const Real rad_y, const Real rad_z) {
  Quatf rot_x(AngleAxis(rad_x, Vec3f::UnitX()));
  Quatf rot_y(AngleAxis(rad_y, Vec3f::UnitY()));
  Quatf rot_z(AngleAxis(rad_z, Vec3f::UnitZ()));

  return rot_y * rot_x * rot_z;
}


// 透視投影行列を生成
// SOURCE:mesa
Affinef perspectiveMatrix(const Real fovy, const Real aspect, const Real zNear, const Real zFar) {
	Real sine, cotangent, deltaZ;
	Real radians = fovy / 2.0;

	deltaZ = zFar - zNear;
	sine = std::sin(radians);
	assert((deltaZ != 0.0) && (sine != 0.0) && (aspect != 0.0));
	cotangent = std::cos(radians) / sine;

	Affinef m(Affinef::Identity());
	m(0,0) = cotangent / aspect;
	m(1,1) = cotangent;
	m(2,2) = -(zFar + zNear) / deltaZ;
	m(2,3) = -2.0 * zNear * zFar / deltaZ;
	m(3,2) = -1.0;
	m(3,3) = 0.0;

  return m;
}


// 3D座標→投影座標
// SOURCE:mesa
std::pair<bool, Vec3f> pointProject(const Vec3f& pos,
                                    const Mat4f& modelMatrix, const Mat4f& projMatrix,
                                    const std::vector<int>& viewport) {
	Vec4f in(pos.x(), pos.y(), pos.z(), 1.0);
	Vec4f out = modelMatrix * in;
	in = projMatrix * out;
	if (in(3) == 0.0) {
    return std::make_pair(false, Vec3f());
  }

	/* Map x, y and z to range 0-1 */
	for (u_int i = 0; i < 3; ++i) {
		Real a = in(i) / in(3);
		in(i) = a * 0.5 + 0.5;
	}

	/* Map x,y to viewport */
	in(0) = in(0) * viewport[2] + viewport[0];
	in(1) = in(1) * viewport[3] + viewport[1];

  return std::make_pair(true, Vec3f(in.x(), in.y(), in.z()));
}

// 投影座標→3D座標
// SOURCE:mesa
std::pair<bool, Vec3f> pointUnProject(const Vec3f& pos,
                                      const Mat4f& modelMatrix, const Mat4f& projMatrix,
                                      const std::vector<int>& viewport) {
	Vec4f in(pos.x(), pos.y(), pos.z(), 1.0);

	/* Map x and y from window coordinates */
	in(0) = (in(0) - viewport[0]) / viewport[2];
	in(1) = (in(1) - viewport[1]) / viewport[3];

	/* Map to range -1 to 1 */
	for (u_int i = 0; i < 3; ++i) {
		in(i) = in(i) * 2.0 - 1.0;
	}

	// 座標に逆行列を掛けて完成
	Mat4f finalMatrix =  projMatrix * modelMatrix;
	Vec4f out = finalMatrix.inverse() * in;
	if (out.w() == 0.0) {
    return std::make_pair(false, Vec3f());
  }

	return std::make_pair(true,
                        Vec3f(out.x() / out.w(), out.y() / out.w(), out.z() / out.w()));
}

}

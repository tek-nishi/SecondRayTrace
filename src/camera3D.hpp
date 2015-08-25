
#pragma once

//
// カメラ(3D向け)
//

#include "defines.hpp"
#include <iostream>
#include <utility>
#include "vector.hpp"
#include "matrix.hpp"


namespace {

class Camera3D {
	Real fovy_;
	Real near_z_;
	Real far_z_;
  
  Vec3f eye_position_;
  Quatf rotate_;

  Affinef projection_;
  Affinef model_;
  
  
public:
	Camera3D(const Real fovy, const Real near_z, const Real far_z) :
    fovy_(fovy),
    near_z_(near_z),
    far_z_(far_z),
    eye_position_(Vec3f::Zero()),
    rotate_(Quatf::Identity()),
    projection_(Affinef::Identity()),
    model_(Affinef::Identity())
	{}
  
  
  void fovy(const Real angle_rad) { fovy_ = angle_rad; }
  Real fovy() const { return fovy_; }

  void nearZ(const Real z) { near_z_ = z; }
  Real nearZ() const { return near_z_; }

  void farZ(const Real z) { far_z_ = z; }
  Real farZ() const { return far_z_; }

  void eyePosition(const Vec3f& position) { eye_position_ = position; }
  const Vec3f& eyePosition() const { return eye_position_; }

  void rotate(const Quatf& rotate) { rotate_ = rotate; }
  const Quatf& rotate() const { return rotate_; }
  
  
  std::pair<Affinef, Affinef> operator()(const Vec2f& view_size) {
    Real aspect = view_size.x() / view_size.y();
    Real fovy   = (aspect < 1.0) ? verticalFovy(view_size) : fovy_;

    projection_ = perspectiveMatrix(fovy, aspect, near_z_, far_z_);
    model_      = Affinef(rotate_ * Translation(eye_position_));

    return std::make_pair(projection_, model_);
  }


  // スクリーン座標→ワールド座標
  // FIXME:１フレーム前の行列で計算している
  Vec3f posToWorld(const Vec3f& pos, const Affinef& model, const std::vector<GLint>& viewport) const {
    // std::vector<GLint> view(4);
		// glGetIntegerv(GL_VIEWPORT, &view[0]);

    Affinef model_camera = model_ * model;
    
		auto world_pos = pointUnProject(pos,
                                    model_camera.matrix(), projection_.matrix(),
                                    viewport);
		return world_pos.second;
	}


private:
  Real verticalFovy(const Vec2f& view_size) const {
    // fovyとnear_zから投影面の幅の半分を求める
    Real half_w = std::tan(fovy_ / 2) * near_z_;

    // 表示画面の縦横比から、投影面の高さの半分を求める
    Real aspect = view_size.y() / view_size.x();
    Real half_h = half_w * aspect;

    // 投影面の高さ半分とnear_zから、縦画面の時のfovyが求まる
    return std::atan(half_h / near_z_) * 2;
  }
  
};

}

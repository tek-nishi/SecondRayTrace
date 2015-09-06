
#pragma once

//
// 衝突判定
// SOURCE:Real-Time Collision Detection by Christer Ericson
//

#include "defines.hpp"
#include <cfloat>
#include "matrix.hpp"


namespace {


// 判定に使う直方体(axis-aligned bounding box)
struct AABBVolume {
	// 中心位置
	Vec3f point;
	// 中心からの距離
	Vec3f radius;
};

// 判定に使う球
struct SphereVolume {
	// 中心位置
	Vec3f point;
	// 半径
	Real radius;
};


// 球aと球bの接触判定
bool testSpheres(const SphereVolume& a, const SphereVolume& b) {
	// aとbの中心点の距離の平方を求める
	Vec3f d = a.point - b.point;
	Real square = d.dot(d);

	Real r = a.radius + b.radius;
	return square <= (r * r);
}

// 点pとAABBとの距離の平方
Real squarePointAABB(const Vec3f& p, const AABBVolume& b) {
	Real square = 0.0;
	for (u_int i = 0; i < 3; ++i) {
		Real p_min = b.point(i) - b.radius(i);
		Real p_max = b.point(i) + b.radius(i);
		Real v = p(i);
		if (v < p_min) square += (p_min - v) * (p_min - v);
		if (v > p_max) square += (v - p_max) * (v - p_max);
	}
	return square;
}

// 点pとAABBとの最接近点
void closestPointAABB(Vec3f& res, const Vec3f& p, const AABBVolume& b) {
	for (u_int i = 0; i < 3; ++i) {
		Real p_min = b.point(i) - b.radius(i);
		Real p_max = b.point(i) + b.radius(i);
		Real v = p(i);
		if (v < p_min) v = p_min;
		if (v > p_max) v = p_max;
		res(i) = v;
	}
}

// 球aとAABBの接触判定
bool testSphereAABB(const SphereVolume& a, const AABBVolume& b) {
	Real square = squarePointAABB(a.point, b);
	return square <= (a.radius * a.radius);
}

// 球aとAABBの接触判定 & 最接近点
bool testSphereAABB(Vec3f& res, const SphereVolume& a, const AABBVolume& b) {
	closestPointAABB(res, a.point, b);
	Vec3f v = res - a.point;
	return v.dot(v) <= (a.radius * a.radius);
}

// 球と光線(p + td)との交差判定
bool testRaySphere(const Vec3f& p, const Vec3f& d, const SphereVolume& s) {
	Vec3f m = p - s.point;

	Real c = m.dot(m) - s.radius * s.radius;
	if (c <= 0.0) return true;

	Real b = m.dot(d);
	if (b > 0.0) return false;

	Real disc = b * b - c;
	if (disc < 0.0) return false;

	return true;
}

// 球と光線(p + td)との交差判定と交差点
// 内包は判定しない
bool testRaySphere(Vec3f& res, Real& res_t, const Vec3f& p, const Vec3f& d, const SphereVolume& s) {
	Vec3f m = p - s.point;

	Real b = m.dot(d);
	Real c = m.dot(m) - s.radius * s.radius;
	if ((c > 0.0) && (b > 0.0)) return false;

	Real disc = b * b - c;
	if (disc < 0.0) return false;

	Real tn = -b - std::sqrt(disc);
  Real tp = -b + std::sqrt(disc);
  // 交差位置が両方とも負の場合は交差しないと判定
  if (std::max(tn, tp) < 0.0) return false;

  // 手前の交差位置を求める
  Real t = tp;
  if (tn > 0.0) t = tn;

  res_t = t;
	res = p + t * d;

	return true;
}

// 球と線分(start, end)との交差判定
bool testLineSphere(Vec3f& res, Real& res_t, const SphereVolume& volume, const Vec3f& pos_start, const Vec3f& pos_end) {
  return testRaySphere(res, res_t,
                       pos_start, Vec3f(pos_end - pos_start).normalized(),
                       volume);
}

// 点p0→p1とAABBとの交差判定
bool testSegmentAABB(const Vec3f& p0, const Vec3f& p1, const AABBVolume& b) {
	Vec3f b_max = b.point + b.radius;
	Vec3f e = b_max - b.point;
	Vec3f m = (p0 + p1) * 0.5;
	Vec3f d = p1 - m;
	m = m - b.point;

	Real adx = std::abs(d.x());
	if (std::abs(m.x()) > e.x() + adx) return false;
	Real ady = std::abs(d.y());
	if (std::abs(m.y()) > e.y() + ady) return false;
	Real adz = std::abs(d.z());
	if (std::abs(m.z()) > e.z() + adz) return false;

	adx += FLT_EPSILON;
	ady += FLT_EPSILON;
	adz += FLT_EPSILON;

	if (std::abs(m.y() * d.z() - m.z() * d.y()) > e.y() * adz + e.z() * ady) return false;
	if (std::abs(m.z() * d.x() - m.x() * d.z()) > e.x() * adz + e.z() * adx) return false;
	if (std::abs(m.x() * d.y() - m.y() * d.x()) > e.x() * ady + e.y() * adx) return false;

	return true;
}

// AABBと光線(p + td)との交差判定と交差点
bool testRayAABB(Vec3f& res, Real& res_t, const Vec3f& p, const Vec3f& d, const AABBVolume& b) {
	Real tmin = 0.0;
	Real tmax = FLT_MAX;

  Vec3f b_min = b.point - b.radius;
  Vec3f b_max = b.point + b.radius;
  
	for (u_int i = 0; i < 3; ++i) {

		if (std::abs(d(i)) < FLT_EPSILON) {
			if (p(i) < b_min(i) || p(i) > b_max(i)) return false;
		}
		else {
			Real ood = 1.0 / d(i);
			Real t1 = (b_min(i) - p(i)) * ood;
			Real t2 = (b_max(i) - p(i)) * ood;

			if (t1 > t2) std::swap(t1, t2);
			if (t1 > tmin) tmin = t1;
			if (t2 < tmax) tmax = t2;
			if (tmin > tmax) return false;
		}
	}
	res = p + d * tmin;
  res_t = tmin;

	return true;
}


struct Plane {
	Vec3f n;
	Real d;
};

// 三点(時計回り)から平面の方程式を求める
Plane computePlane(const Vec3f& a, const Vec3f& b, const Vec3f& c) {
	Plane p;

	p.n = ((b - a).cross(c - a)).normalized();
	p.d = p.n.dot(a);
	
	return p;
}

// 点qと平面pとの距離を求める
// マイナスの場合、点は平面の下側にある
Real distPointPlane(const Vec3f& q, const Plane& p) {
	return (p.n.dot(q) - p.d) / p.n.dot(p.n);
}


struct Box {
  // 左上座標
  Vec2f inf;
  // 右下座標
  Vec2f sup;
};

// 点と矩形
bool testPointBox(const Vec2f& pos, const Box& box) {
  return pos.x() >= box.inf.x()
      && pos.x() <= box.sup.x()
      && pos.y() >= box.inf.y()
      && pos.y() <= box.sup.y();
}


struct Triangle {
  Vec3f a, b, c;

  // Vec3f ab;
  // Vec3f ac;
  // Vec3f n;
};


#if 0

Triangle makeTriangle(const Vec3f& a, const Vec3f& b, const Vec3f& c) {
  Vec3f ab = b - a;
  Vec3f ac = c - a;
  
  return { a, b, c, ab, ac, ab.cross(ac) };
}

#endif


bool testRayTriangle(Vec3f& res, Real& t, Vec3f& n, Vec3f& center,
                     const Vec3f& p, const Vec3f& d,
                     const Triangle& tri, const bool back_face = false) {
  Vec3f ab = tri.b - tri.a;
  Vec3f ac = tri.c - tri.a;
  // Vec3f ab = tri.ab;
  // Vec3f ac = tri.ac;
  Vec3f qp = -d;

  bool back = false;
  
  n = ab.cross(ac);
  // n = tri.n;
  
  Real dt = qp.dot(n);
  if (back_face && (dt <= 0.0)) {
    // 裏面も対象にする
    std::swap(ab, ac);
    n = -n;
    dt = qp.dot(n);
    back = true;
  }
  if (dt <= 0.0) return false;

  Vec3f ap = p - tri.a;
  t = ap.dot(n);
  if (t < 0.0) return false;

  Vec3f e = qp.cross(ap);
  Real v = ac.dot(e);
  if (v < 0.0 || v > dt) return false;

  Real w = -ab.dot(e);
  if (w < 0.0 || (v + w) > dt) return false;
  
  Real ood = 1.0 / dt;
  t *= ood;
  v *= ood;
  w *= ood;
  Real u = 1.0 - v - w;

  if (back) {
    n = -n;
    std::swap(v, w);
  }
  
  res << tri.a * u + tri.b * v + tri.c * w;
  center << u, v, w;

  return true;
}

}

/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2012 Ausenco Engineering Canada Inc.
 * Copyright (C) 2023 JaamSim Software Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <cmath>
#include <stdexcept>
#include <string>

#include "math_utils.h"
#include "vec3d.h"
#include "vec4d.h"

namespace jaamsim::math {

struct Quaternion {
	double x;
	double y;
	double z;
	double w;

	constexpr Quaternion() noexcept : x{0.0}, y{0.0}, z{0.0}, w{1.0} {}
	constexpr Quaternion(double ix, double iy, double iz, double r) noexcept
		: x{ix}, y{iy}, z{iz}, w{r} {}
	constexpr Quaternion(const Quaternion& q) noexcept
		: x{q.x}, y{q.y}, z{q.z}, w{q.w} {}

	// --- set ---
	constexpr void set(const Quaternion& q) noexcept {
		x = q.x; y = q.y; z = q.z; w = q.w;
	}

	// --- set from Euler angles (global X, then Y, then Z) ---
	void set_euler3(const Vec3d& v) {
		Quaternion tmp;
		set_rot_x_axis(v.x);
		tmp.set_rot_y_axis(v.y);
		mult(tmp, *this);
		tmp.set_rot_z_axis(v.z);
		mult(tmp, *this);
	}

	// --- get Euler angles ---
	Vec3d get_euler3() const {
		Vec3d ret;
		ret.x = std::atan2(2.0 * (w * x + y * z), 1.0 - 2.0 * (x * x + y * y));
		double val = 2.0 * (w * y - x * z);
		ret.y = -3.14159265358979323846 / 2.0
		      + 2.0 * std::atan2(std::sqrt(1.0 + val), std::sqrt(1.0 - val));
		ret.z = std::atan2(2.0 * (w * z + x * y), 1.0 - 2.0 * (y * y + z * z));
		return ret;
	}

	// --- axis-angle rotations ---
	void set_rot_x_axis(double angle) noexcept {
		double half = 0.5 * angle;
		x = std::sin(half); y = 0.0; z = 0.0; w = std::cos(half);
	}
	void set_rot_y_axis(double angle) noexcept {
		double half = 0.5 * angle;
		x = 0.0; y = std::sin(half); z = 0.0; w = std::cos(half);
	}
	void set_rot_z_axis(double angle) noexcept {
		double half = 0.5 * angle;
		x = 0.0; y = 0.0; z = std::sin(half); w = std::cos(half);
	}
	void set_axis_angle(const Vec3d& axis, double angle) {
		double half = 0.5 * angle;
		Vec3d v{axis};
		v.normalize3();
		v.scale3(std::sin(half));
		x = v.x; y = v.y; z = v.z;
		w = std::cos(half);
	}

	// --- factory: rotation between two unit vectors ---
	static Quaternion transform_vectors(const Vec4d& from, const Vec4d& to) {
		Vec4d f{from};
		Vec4d t{to};
		f.normalize3();
		t.normalize3();
		Vec4d cross{0.0, 0.0, 0.0, 1.0};
		cross.cross3(f, t);
		double angle = std::asin(cross.mag3());
		cross.normalize3();
		Quaternion ret;
		ret.set_axis_angle(cross, angle);
		return ret;
	}

	// --- magnitude ---
	constexpr double mag_squared() const noexcept { return dot(*this); }
	double mag() const noexcept { return std::sqrt(mag_squared()); }

	// --- dot ---
	static constexpr double dot(const Quaternion& q1, const Quaternion& q2) noexcept {
		return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
	}
	constexpr double dot(const Quaternion& q) const noexcept {
		return dot(*this, q);
	}

	// --- normalize ---
	void normalize() noexcept { normalize(*this); }
	void normalize(const Quaternion& q) noexcept {
		double m = dot(q, q);
		if (is_small(m)) {
			x = 0.0; y = 0.0; z = 0.0; w = 1.0; return;
		}
		m = std::sqrt(m);
		x = q.x / m; y = q.y / m; z = q.z / m; w = q.w / m;
	}

	// --- add ---
	void add(const Quaternion& q) noexcept {
		x += q.x; y += q.y; z += q.z; w += q.w;
	}

	// --- scale ---
	void scale(double s) noexcept {
		x *= s; y *= s; z *= s; w *= s;
	}

	// --- conjugate ---
	void conjugate() noexcept {
		x = -x; y = -y; z = -z;
	}
	void conjugate(const Quaternion& q) noexcept {
		x = -q.x; y = -q.y; z = -q.z; w = q.w;
	}

	// --- multiplication (composition of rotations) ---
	void mult(const Quaternion& a, const Quaternion& b) noexcept {
		double _x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
		double _y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
		double _z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
		double _w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
		x = _x; y = _y; z = _z; w = _w;
	}

	// --- lerp (linear interpolation) ---
	void lerp(const Quaternion& q, double weight, Quaternion& res) const noexcept {
		double w1 = 1.0 - weight;
		res.x = x * w1 + q.x * weight;
		res.y = y * w1 + q.y * weight;
		res.z = z * w1 + q.z * weight;
		res.w = w * w1 + q.w * weight;
	}

	// --- slerp (spherical linear interpolation) ---
	void slerp(const Quaternion& q, double weight, Quaternion& res) const {
		double cos_theta = dot(q);
		if (cos_theta > 0.95) {
			lerp(q, weight, res);
			res.normalize();
			return;
		}
		double theta = std::acos(cos_theta);
		double sin_theta = std::sin(theta);
		if (is_small(sin_theta)) {
			throw std::invalid_argument("Cannot slerp two opposite quaternions");
		}
		double this_scale = std::sin((1.0 - weight) * theta) / sin_theta;
		double q_scale = std::sin(weight * theta) / sin_theta;
		res.x = x * this_scale + q.x * q_scale;
		res.y = y * this_scale + q.y * q_scale;
		res.z = z * this_scale + q.z * q_scale;
		res.w = w * this_scale + q.w * q_scale;
	}

	// --- validation ---
	bool is_normal() const noexcept {
		return near(mag_squared(), 1.0);
	}

	// --- equality ---
	constexpr bool equals(const Quaternion& q) const noexcept {
		return x == q.x && y == q.y && z == q.z && w == q.w;
	}
	bool near(const Quaternion& q) const noexcept {
		return math::near(x, q.x)
		    && math::near(y, q.y)
		    && math::near(z, q.z)
		    && math::near(w, q.w);
	}

	// --- operator overloads ---
	friend constexpr Quaternion operator+(const Quaternion& a, const Quaternion& b) noexcept {
		return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
	}
	friend constexpr Quaternion operator*(const Quaternion& a, const Quaternion& b) noexcept {
		Quaternion r;
		r.mult(a, b);
		return r;
	}
	friend constexpr Quaternion operator*(const Quaternion& q, double s) noexcept {
		return {q.x * s, q.y * s, q.z * s, q.w * s};
	}
	friend constexpr Quaternion operator*(double s, const Quaternion& q) noexcept {
		return {q.x * s, q.y * s, q.z * s, q.w * s};
	}
	Quaternion& operator+=(const Quaternion& q) noexcept { add(q); return *this; }
	Quaternion& operator*=(const Quaternion& q) noexcept { mult(q, *this); return *this; }
	Quaternion& operator*=(double s) noexcept { scale(s); return *this; }

	friend constexpr bool operator==(const Quaternion& a, const Quaternion& b) noexcept {
		return a.equals(b);
	}
	friend constexpr bool operator!=(const Quaternion& a, const Quaternion& b) noexcept {
		return !a.equals(b);
	}

	std::string to_string() const {
		return "[(" + std::to_string(x) + ", " + std::to_string(y) + ", "
		     + std::to_string(z) + ")i, " + std::to_string(w) + "]";
	}
};

}  // namespace jaamsim::math

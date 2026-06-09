/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2012 Ausenco Engineering Canada Inc.
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

#include <algorithm>
#include <cmath>
#include <string>

#include "math_utils.h"
#include "vec2d.h"

namespace jaamsim::math {

struct Mat4d;     // forward declaration
struct Quaternion;

struct Vec3d {
	double x;
	double y;
	double z;

	constexpr Vec3d() noexcept : x{0.0}, y{0.0}, z{0.0} {}
	constexpr Vec3d(double x, double y, double z) noexcept : x{x}, y{y}, z{z} {}
	constexpr Vec3d(const Vec3d& v) noexcept : x{v.x}, y{v.y}, z{v.z} {}

	// --- set ---
	constexpr void set3(const Vec3d& v) noexcept { x = v.x; y = v.y; z = v.z; }
	constexpr void set3(double nx, double ny, double nz) noexcept { x = nx; y = ny; z = nz; }

	// --- add ---
	void add3(const Vec3d& v) noexcept { x += v.x; y += v.y; z += v.z; }
	void add3(const Vec3d& v1, const Vec3d& v2) noexcept {
		x = v1.x + v2.x; y = v1.y + v2.y; z = v1.z + v2.z;
	}

	// --- sub ---
	void sub3(const Vec3d& v) noexcept { x -= v.x; y -= v.y; z -= v.z; }
	void sub3(const Vec3d& v1, const Vec3d& v2) noexcept {
		x = v1.x - v2.x; y = v1.y - v2.y; z = v1.z - v2.z;
	}

	// --- mul (component-wise) ---
	void mul3(const Vec3d& v) noexcept { x *= v.x; y *= v.y; z *= v.z; }
	void mul3(const Vec3d& v1, const Vec3d& v2) noexcept {
		x = v1.x * v2.x; y = v1.y * v2.y; z = v1.z * v2.z;
	}

	// --- min ---
	void min3(const Vec3d& v) noexcept {
		x = std::fmin(x, v.x); y = std::fmin(y, v.y); z = std::fmin(z, v.z);
	}
	void min3(const Vec3d& v1, const Vec3d& v2) noexcept {
		x = std::fmin(v1.x, v2.x); y = std::fmin(v1.y, v2.y); z = std::fmin(v1.z, v2.z);
	}

	// --- max ---
	void max3(const Vec3d& v) noexcept {
		x = std::fmax(x, v.x); y = std::fmax(y, v.y); z = std::fmax(z, v.z);
	}
	void max3(const Vec3d& v1, const Vec3d& v2) noexcept {
		x = std::fmax(v1.x, v2.x); y = std::fmax(v1.y, v2.y); z = std::fmax(v1.z, v2.z);
	}

	// --- dot ---
	static constexpr double dot3(const Vec3d& v1, const Vec3d& v2) noexcept {
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}
	constexpr double dot3(const Vec3d& v) const noexcept {
		return dot3(*this, v);
	}

	// --- magnitude ---
	double mag3() const noexcept { return std::sqrt(dot3(*this, *this)); }
	constexpr double mag_square3() const noexcept { return dot3(*this, *this); }

	// --- normalize ---
	void normalize3() noexcept { normalize3(*this); }
	void normalize3(const Vec3d& v) noexcept {
		double mag = dot3(v, v);
		if (non_normal_mag(mag)) {
			x = 0.0; y = 0.0; z = 1.0; return;
		}
		mag = std::sqrt(mag);
		x = v.x / mag; y = v.y / mag; z = v.z / mag;
	}

	// --- scale ---
	void scale3(double scale) noexcept { x *= scale; y *= scale; z *= scale; }
	void scale3(double scale, const Vec3d& v) noexcept {
		x = v.x * scale; y = v.y * scale; z = v.z * scale;
	}

	// --- interpolate ---
	void interpolate3(const Vec3d& a, const Vec3d& b, double ratio) noexcept {
		double temp = 1.0 - ratio;
		x = temp * a.x + ratio * b.x;
		y = temp * a.y + ratio * b.y;
		z = temp * a.z + ratio * b.z;
	}

	// --- slerp (spherical linear interpolation) ---
	void slerp(const Vec3d& a, const Vec3d& b, double ratio) {
		double cos_theta = a.dot3(b);
		if (cos_theta > 0.95) {
			interpolate3(a, b, ratio);
			normalize3();
			return;
		}
		cos_theta = std::fmax(std::fmin(cos_theta, 1.0), 0.0);
		double theta = std::acos(cos_theta);
		double sin_theta = std::sin(theta);
		double weight0 = std::sin((1.0 - ratio) * theta) / sin_theta;
		double weight1 = std::sin(ratio * theta) / sin_theta;
		x = weight0 * a.x + weight1 * b.x;
		y = weight0 * a.y + weight1 * b.y;
		z = weight0 * a.z + weight1 * b.z;
	}

	// --- mult3: matrix transform (no translation) ---
	void mult3(const Mat4d& m, const Vec3d& v);
	void mult3(const Vec3d& v, const Mat4d& m);

	// --- mult_and_trans3: matrix transform with translation ---
	void mult_and_trans3(const Mat4d& m, const Vec3d& v);

	// --- cross product ---
	void cross3(const Vec3d& v) noexcept {
		double _x = y * v.z - z * v.y;
		double _y = z * v.x - x * v.z;
		double _z = x * v.y - y * v.x;
		x = _x; y = _y; z = _z;
	}
	void cross3(const Vec3d& v1, const Vec3d& v2) noexcept {
		x = v1.y * v2.z - v1.z * v2.y;
		y = v1.z * v2.x - v1.x * v2.z;
		z = v1.x * v2.y - v1.y * v2.x;
	}

	// --- equality ---
	constexpr bool equals3(const Vec3d& v) const noexcept {
		return x == v.x && y == v.y && z == v.z;
	}
	bool near3(const Vec3d& v) const noexcept {
		return near(x, v.x) && near(y, v.y) && near(z, v.z);
	}

	// --- operator overloads ---
	friend constexpr Vec3d operator+(const Vec3d& a, const Vec3d& b) noexcept {
		return {a.x + b.x, a.y + b.y, a.z + b.z};
	}
	friend constexpr Vec3d operator-(const Vec3d& a, const Vec3d& b) noexcept {
		return {a.x - b.x, a.y - b.y, a.z - b.z};
	}
	friend constexpr Vec3d operator*(const Vec3d& a, const Vec3d& b) noexcept {
		return {a.x * b.x, a.y * b.y, a.z * b.z};
	}
	friend constexpr Vec3d operator*(const Vec3d& v, double s) noexcept {
		return {v.x * s, v.y * s, v.z * s};
	}
	friend constexpr Vec3d operator*(double s, const Vec3d& v) noexcept {
		return {v.x * s, v.y * s, v.z * s};
	}
	friend constexpr Vec3d operator/(const Vec3d& v, double s) noexcept {
		return {v.x / s, v.y / s, v.z / s};
	}
	friend constexpr Vec3d operator-(const Vec3d& v) noexcept {
		return {-v.x, -v.y, -v.z};
	}
	Vec3d& operator+=(const Vec3d& v) noexcept { add3(v); return *this; }
	Vec3d& operator-=(const Vec3d& v) noexcept { sub3(v); return *this; }
	Vec3d& operator*=(double s) noexcept { scale3(s); return *this; }
	Vec3d& operator/=(double s) noexcept { x /= s; y /= s; z /= s; return *this; }

	friend constexpr bool operator==(const Vec3d& a, const Vec3d& b) noexcept {
		return a.equals3(b);
	}
	friend constexpr bool operator!=(const Vec3d& a, const Vec3d& b) noexcept {
		return !a.equals3(b);
	}

	std::string to_string() const {
		return std::to_string(x) + "  " + std::to_string(y) + "  " + std::to_string(z);
	}
};

}  // namespace jaamsim::math

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
#include "vec3d.h"

namespace jaamsim::math {

struct Mat4d;  // forward declaration

struct Vec4d {
	double x;
	double y;
	double z;
	double w;

	constexpr Vec4d() noexcept : x{0.0}, y{0.0}, z{0.0}, w{0.0} {}
	constexpr Vec4d(double x, double y, double z, double w) noexcept : x{x}, y{y}, z{z}, w{w} {}
	constexpr Vec4d(const Vec4d& v) noexcept : x{v.x}, y{v.y}, z{v.z}, w{v.w} {}
	constexpr Vec4d(const Vec3d& v, double w) noexcept : x{v.x}, y{v.y}, z{v.z}, w{w} {}

	// --- set ---
	constexpr void set4(const Vec4d& v) noexcept { x = v.x; y = v.y; z = v.z; w = v.w; }
	constexpr void set4(double nx, double ny, double nz, double nw) noexcept { x = nx; y = ny; z = nz; w = nw; }

	// --- add ---
	void add4(const Vec4d& v) noexcept { x += v.x; y += v.y; z += v.z; w += v.w; }
	void add4(const Vec4d& v1, const Vec4d& v2) noexcept {
		x = v1.x + v2.x; y = v1.y + v2.y; z = v1.z + v2.z; w = v1.w + v2.w;
	}

	// --- sub ---
	void sub4(const Vec4d& v) noexcept { x -= v.x; y -= v.y; z -= v.z; w -= v.w; }
	void sub4(const Vec4d& v1, const Vec4d& v2) noexcept {
		x = v1.x - v2.x; y = v1.y - v2.y; z = v1.z - v2.z; w = v1.w - v2.w;
	}

	// --- mul (component-wise) ---
	void mul4(const Vec4d& v) noexcept { x *= v.x; y *= v.y; z *= v.z; w *= v.w; }
	void mul4(const Vec4d& v1, const Vec4d& v2) noexcept {
		x = v1.x * v2.x; y = v1.y * v2.y; z = v1.z * v2.z; w = v1.w * v2.w;
	}

	// --- min ---
	void min4(const Vec4d& v) noexcept {
		x = std::fmin(x, v.x); y = std::fmin(y, v.y); z = std::fmin(z, v.z); w = std::fmin(w, v.w);
	}
	void min4(const Vec4d& v1, const Vec4d& v2) noexcept {
		x = std::fmin(v1.x, v2.x); y = std::fmin(v1.y, v2.y); z = std::fmin(v1.z, v2.z); w = std::fmin(v1.w, v2.w);
	}

	// --- max ---
	void max4(const Vec4d& v) noexcept {
		x = std::fmax(x, v.x); y = std::fmax(y, v.y); z = std::fmax(z, v.z); w = std::fmax(w, v.w);
	}
	void max4(const Vec4d& v1, const Vec4d& v2) noexcept {
		x = std::fmax(v1.x, v2.x); y = std::fmax(v1.y, v2.y); z = std::fmax(v1.z, v2.z); w = std::fmax(v1.w, v2.w);
	}

	// --- dot ---
	static constexpr double dot4(const Vec4d& v1, const Vec4d& v2) noexcept {
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
	}
	constexpr double dot4(const Vec4d& v) const noexcept {
		return dot4(*this, v);
	}

	// --- magnitude ---
	double mag4() const noexcept { return std::sqrt(dot4(*this, *this)); }
	constexpr double mag_square4() const noexcept { return dot4(*this, *this); }

	// Convenience: 2-component mag (used by MathUtils ray-space calculations)
	double mag2() const noexcept { return std::sqrt(x * x + y * y); }
	constexpr double mag_square2() const noexcept { return x * x + y * y; }
	constexpr double dot2(const Vec4d& v) const noexcept { return x * v.x + y * v.y; }

	// --- normalize ---
	void normalize4() noexcept { normalize4(*this); }
	void normalize4(const Vec4d& v) noexcept {
		double mag = dot4(v, v);
		if (non_normal_mag(mag)) {
			x = 0.0; y = 0.0; z = 0.0; w = 1.0; return;
		}
		mag = std::sqrt(mag);
		x = v.x / mag; y = v.y / mag; z = v.z / mag; w = v.w / mag;
	}

	// Convenience: 3-component normalize (Vec4d.normalize3 in Java)
	void normalize3() noexcept {
		double mag = x * x + y * y + z * z;
		if (non_normal_mag(mag)) {
			x = 0.0; y = 0.0; z = 1.0; return;
		}
		mag = std::sqrt(mag);
		x /= mag; y /= mag; z /= mag;
	}

	// Convenience: 3-component cross (cross3 from Vec4d) used by Quaternion::transformVectors
	void cross3(const Vec4d& v1, const Vec4d& v2) noexcept {
		x = v1.y * v2.z - v1.z * v2.y;
		y = v1.z * v2.x - v1.x * v2.z;
		z = v1.x * v2.y - v1.y * v2.x;
	}

	// --- scale ---
	void scale4(double scale) noexcept { x *= scale; y *= scale; z *= scale; w *= scale; }
	void scale4(double scale, const Vec4d& v) noexcept {
		x = v.x * scale; y = v.y * scale; z = v.z * scale; w = v.w * scale;
	}

	// Convenience: 3-component sub and scale (Vec4d.sub3, scale3), used by MathUtils
	void sub3(const Vec4d& v) noexcept { x -= v.x; y -= v.y; z -= v.z; }
	void sub3(const Vec4d& v1, const Vec4d& v2) noexcept {
		x = v1.x - v2.x; y = v1.y - v2.y; z = v1.z - v2.z;
	}
	void scale3(double scale) noexcept { x *= scale; y *= scale; z *= scale; }
	void scale3(double scale, const Vec4d& v) noexcept {
		x = v.x * scale; y = v.y * scale; z = v.z * scale;
	}
	void add3(const Vec4d& v) noexcept { x += v.x; y += v.y; z += v.z; }

	// --- interpolate ---
	void interpolate4(const Vec4d& a, const Vec4d& b, double ratio) noexcept {
		double temp = 1.0 - ratio;
		x = temp * a.x + ratio * b.x;
		y = temp * a.y + ratio * b.y;
		z = temp * a.z + ratio * b.z;
		w = temp * a.w + ratio * b.w;
	}

	// --- mult4: full 4x4 matrix transform ---
	void mult4(const Mat4d& m, const Vec4d& v);
	void mult4(const Vec4d& v, const Mat4d& m);

	// --- index access ---
	void set_by_ind(int index, double val) noexcept {
		switch (index) {
		case 0: x = val; return;
		case 1: y = val; return;
		case 2: z = val; return;
		case 3: w = val; return;
		}
	}
	constexpr double get_by_ind(int index) const noexcept {
		switch (index) {
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
		}
		return 0.0;
	}

	// --- equality ---
	constexpr bool equals4(const Vec4d& v) const noexcept {
		return x == v.x && y == v.y && z == v.z && w == v.w;
	}
	bool near4(const Vec4d& v) const noexcept {
		return near(x, v.x) && near(y, v.y) && near(z, v.z) && near(w, v.w);
	}

	// --- operator overloads ---
	friend constexpr Vec4d operator+(const Vec4d& a, const Vec4d& b) noexcept {
		return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
	}
	friend constexpr Vec4d operator-(const Vec4d& a, const Vec4d& b) noexcept {
		return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
	}
	friend constexpr Vec4d operator*(const Vec4d& a, const Vec4d& b) noexcept {
		return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
	}
	friend constexpr Vec4d operator*(const Vec4d& v, double s) noexcept {
		return {v.x * s, v.y * s, v.z * s, v.w * s};
	}
	friend constexpr Vec4d operator*(double s, const Vec4d& v) noexcept {
		return {v.x * s, v.y * s, v.z * s, v.w * s};
	}
	friend constexpr Vec4d operator/(const Vec4d& v, double s) noexcept {
		return {v.x / s, v.y / s, v.z / s, v.w / s};
	}
	friend constexpr Vec4d operator-(const Vec4d& v) noexcept {
		return {-v.x, -v.y, -v.z, -v.w};
	}
	Vec4d& operator+=(const Vec4d& v) noexcept { add4(v); return *this; }
	Vec4d& operator-=(const Vec4d& v) noexcept { sub4(v); return *this; }
	Vec4d& operator*=(double s) noexcept { scale4(s); return *this; }
	Vec4d& operator/=(double s) noexcept { x /= s; y /= s; z /= s; w /= s; return *this; }

	friend constexpr bool operator==(const Vec4d& a, const Vec4d& b) noexcept {
		return a.equals4(b);
	}
	friend constexpr bool operator!=(const Vec4d& a, const Vec4d& b) noexcept {
		return !a.equals4(b);
	}

	std::string to_string() const {
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ", "
		     + std::to_string(z) + ", " + std::to_string(w) + ")";
	}
};

}  // namespace jaamsim::math

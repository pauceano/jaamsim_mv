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

#include <cmath>
#include <string>

#include "math_utils.h"

namespace jaamsim::math {

struct Mat4d;  // forward declaration

struct Vec2d {
	double x;
	double y;

	constexpr Vec2d() noexcept : x{0.0}, y{0.0} {}
	constexpr Vec2d(double x, double y) noexcept : x{x}, y{y} {}
	constexpr Vec2d(const Vec2d& v) noexcept : x{v.x}, y{v.y} {}

	// --- set ---
	constexpr void set2(const Vec2d& v) noexcept { x = v.x; y = v.y; }
	constexpr void set2(double nx, double ny) noexcept { x = nx; y = ny; }

	// --- add ---
	void add2(const Vec2d& v) noexcept { x += v.x; y += v.y; }
	void add2(const Vec2d& v1, const Vec2d& v2) noexcept {
		x = v1.x + v2.x; y = v1.y + v2.y;
	}

	// --- sub ---
	void sub2(const Vec2d& v) noexcept { x -= v.x; y -= v.y; }
	void sub2(const Vec2d& v1, const Vec2d& v2) noexcept {
		x = v1.x - v2.x; y = v1.y - v2.y;
	}

	// --- mul (component-wise) ---
	void mul2(const Vec2d& v) noexcept { x *= v.x; y *= v.y; }
	void mul2(const Vec2d& v1, const Vec2d& v2) noexcept {
		x = v1.x * v2.x; y = v1.y * v2.y;
	}

	// --- min ---
	void min2(const Vec2d& v) noexcept {
		x = std::fmin(x, v.x); y = std::fmin(y, v.y);
	}
	void min2(const Vec2d& v1, const Vec2d& v2) noexcept {
		x = std::fmin(v1.x, v2.x); y = std::fmin(v1.y, v2.y);
	}

	// --- max ---
	void max2(const Vec2d& v) noexcept {
		x = std::fmax(x, v.x); y = std::fmax(y, v.y);
	}
	void max2(const Vec2d& v1, const Vec2d& v2) noexcept {
		x = std::fmax(v1.x, v2.x); y = std::fmax(v1.y, v2.y);
	}

	// --- dot ---
	static constexpr double dot2(const Vec2d& v1, const Vec2d& v2) noexcept {
		return v1.x * v2.x + v1.y * v2.y;
	}
	constexpr double dot2(const Vec2d& v) const noexcept {
		return dot2(*this, v);
	}

	// --- magnitude ---
	double mag2() const noexcept { return std::sqrt(dot2(*this, *this)); }
	constexpr double mag_square2() const noexcept { return dot2(*this, *this); }

	// --- normalize ---
	void normalize2() noexcept { normalize2(*this); }
	void normalize2(const Vec2d& v) noexcept {
		double mag = dot2(v, v);
		if (non_normal_mag(mag)) {
			x = 0.0; y = 1.0; return;
		}
		mag = std::sqrt(mag);
		x = v.x / mag; y = v.y / mag;
	}

	// --- scale ---
	void scale2(double scale) noexcept { x *= scale; y *= scale; }
	void scale2(double scale, const Vec2d& v) noexcept {
		x = v.x * scale; y = v.y * scale;
	}

	// --- interpolate ---
	void interpolate2(const Vec2d& a, const Vec2d& b, double ratio) noexcept {
		double temp = 1.0 - ratio;
		x = temp * a.x + ratio * b.x;
		y = temp * a.y + ratio * b.y;
	}

	// --- mult (matrix transform, no translation) ---
	void mult2(const Mat4d& m, const Vec2d& v);
	void mult2(const Vec2d& v, const Mat4d& m);

	// --- equality ---
	constexpr bool equals2(const Vec2d& v) const noexcept {
		return x == v.x && y == v.y;
	}
	bool near2(const Vec2d& v) const noexcept {
		return near(x, v.x) && near(y, v.y);
	}

	// --- operator overloads ---
	friend constexpr Vec2d operator+(const Vec2d& a, const Vec2d& b) noexcept {
		return {a.x + b.x, a.y + b.y};
	}
	friend constexpr Vec2d operator-(const Vec2d& a, const Vec2d& b) noexcept {
		return {a.x - b.x, a.y - b.y};
	}
	friend constexpr Vec2d operator*(const Vec2d& a, const Vec2d& b) noexcept {
		return {a.x * b.x, a.y * b.y};
	}
	friend constexpr Vec2d operator*(const Vec2d& v, double s) noexcept {
		return {v.x * s, v.y * s};
	}
	friend constexpr Vec2d operator*(double s, const Vec2d& v) noexcept {
		return {v.x * s, v.y * s};
	}
	friend constexpr Vec2d operator/(const Vec2d& v, double s) noexcept {
		return {v.x / s, v.y / s};
	}
	friend constexpr Vec2d operator-(const Vec2d& v) noexcept {
		return {-v.x, -v.y};
	}
	Vec2d& operator+=(const Vec2d& v) noexcept { add2(v); return *this; }
	Vec2d& operator-=(const Vec2d& v) noexcept { sub2(v); return *this; }
	Vec2d& operator*=(double s) noexcept { scale2(s); return *this; }
	Vec2d& operator/=(double s) noexcept { x /= s; y /= s; return *this; }

	friend constexpr bool operator==(const Vec2d& a, const Vec2d& b) noexcept {
		return a.equals2(b);
	}
	friend constexpr bool operator!=(const Vec2d& a, const Vec2d& b) noexcept {
		return !a.equals2(b);
	}

	std::string to_string() const {
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}
};

}  // namespace jaamsim::math

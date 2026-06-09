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

#include <array>
#include <cmath>
#include <stdexcept>
#include <string>

#include "math_utils.h"
#include "vec2d.h"
#include "vec3d.h"
#include "vec4d.h"

namespace jaamsim::math {

struct Quaternion;  // forward declaration

class Mat4d {
public:
	double d00, d01, d02, d03;
	double d10, d11, d12, d13;
	double d20, d21, d22, d23;
	double d30, d31, d32, d33;

	// --- constructors ---
	constexpr Mat4d() noexcept
		: d00{1.0}, d01{0.0}, d02{0.0}, d03{0.0}
		, d10{0.0}, d11{1.0}, d12{0.0}, d13{0.0}
		, d20{0.0}, d21{0.0}, d22{1.0}, d23{0.0}
		, d30{0.0}, d31{0.0}, d32{0.0}, d33{1.0} {}

	constexpr Mat4d(const Mat4d& m) noexcept = default;
	Mat4d& operator=(const Mat4d& m) noexcept = default;

	constexpr Mat4d(const std::array<double, 16>& mat) noexcept
		: d00{mat[ 0]}, d01{mat[ 1]}, d02{mat[ 2]}, d03{mat[ 3]}
		, d10{mat[ 4]}, d11{mat[ 5]}, d12{mat[ 6]}, d13{mat[ 7]}
		, d20{mat[ 8]}, d21{mat[ 9]}, d22{mat[10]}, d23{mat[11]}
		, d30{mat[12]}, d31{mat[13]}, d32{mat[14]}, d33{mat[15]} {}

	// --- identity / zero ---
	constexpr void identity() noexcept {
		d00 = 1.0; d01 = 0.0; d02 = 0.0; d03 = 0.0;
		d10 = 0.0; d11 = 1.0; d12 = 0.0; d13 = 0.0;
		d20 = 0.0; d21 = 0.0; d22 = 1.0; d23 = 0.0;
		d30 = 0.0; d31 = 0.0; d32 = 0.0; d33 = 1.0;
	}
	constexpr void zero() noexcept {
		d00 = 0.0; d01 = 0.0; d02 = 0.0; d03 = 0.0;
		d10 = 0.0; d11 = 0.0; d12 = 0.0; d13 = 0.0;
		d20 = 0.0; d21 = 0.0; d22 = 0.0; d23 = 0.0;
		d30 = 0.0; d31 = 0.0; d32 = 0.0; d33 = 0.0;
	}

	// --- set ---
	constexpr void set4(const Mat4d& m) noexcept {
		d00 = m.d00; d01 = m.d01; d02 = m.d02; d03 = m.d03;
		d10 = m.d10; d11 = m.d11; d12 = m.d12; d13 = m.d13;
		d20 = m.d20; d21 = m.d21; d22 = m.d22; d23 = m.d23;
		d30 = m.d30; d31 = m.d31; d32 = m.d32; d33 = m.d33;
	}

	// --- transpose ---
	void transpose4() noexcept {
		double tmp;
		tmp = d01; d01 = d10; d10 = tmp;
		tmp = d02; d02 = d20; d20 = tmp;
		tmp = d03; d03 = d30; d30 = tmp;
		tmp = d12; d12 = d21; d21 = tmp;
		tmp = d13; d13 = d31; d31 = tmp;
		tmp = d23; d23 = d32; d32 = tmp;
	}
	void transpose4(const Mat4d& m) noexcept {
		d00 = m.d00; d11 = m.d11; d22 = m.d22; d33 = m.d33;
		double tmp;
		tmp = m.d01; d01 = m.d10; d10 = tmp;
		tmp = m.d02; d02 = m.d20; d20 = tmp;
		tmp = m.d03; d03 = m.d30; d30 = tmp;
		tmp = m.d12; d12 = m.d21; d21 = tmp;
		tmp = m.d13; d13 = m.d31; d31 = tmp;
		tmp = m.d23; d23 = m.d32; d32 = tmp;
	}

	// --- 3x3 multiplication (upper-left, no translation) ---
	void mult3(const Mat4d& m) noexcept { mult3(*this, m); }
	void mult3(const Mat4d& m1, const Mat4d& m2) noexcept {
		double _d00 = m1.d00 * m2.d00 + m1.d01 * m2.d10 + m1.d02 * m2.d20;
		double _d01 = m1.d00 * m2.d01 + m1.d01 * m2.d11 + m1.d02 * m2.d21;
		double _d02 = m1.d00 * m2.d02 + m1.d01 * m2.d12 + m1.d02 * m2.d22;

		double _d10 = m1.d10 * m2.d00 + m1.d11 * m2.d10 + m1.d12 * m2.d20;
		double _d11 = m1.d10 * m2.d01 + m1.d11 * m2.d11 + m1.d12 * m2.d21;
		double _d12 = m1.d10 * m2.d02 + m1.d11 * m2.d12 + m1.d12 * m2.d22;

		double _d20 = m1.d20 * m2.d00 + m1.d21 * m2.d10 + m1.d22 * m2.d20;
		double _d21 = m1.d20 * m2.d01 + m1.d21 * m2.d11 + m1.d22 * m2.d21;
		double _d22 = m1.d20 * m2.d02 + m1.d21 * m2.d12 + m1.d22 * m2.d22;

		d00 = _d00; d01 = _d01; d02 = _d02;
		d10 = _d10; d11 = _d11; d12 = _d12;
		d20 = _d20; d21 = _d21; d22 = _d22;
	}

	// --- 4x4 multiplication ---
	void mult4(const Mat4d& m) noexcept { mult4(*this, m); }
	void mult4(const Mat4d& m1, const Mat4d& m2) noexcept {
		double _d00 = m1.d00 * m2.d00 + m1.d01 * m2.d10 + m1.d02 * m2.d20 + m1.d03 * m2.d30;
		double _d01 = m1.d00 * m2.d01 + m1.d01 * m2.d11 + m1.d02 * m2.d21 + m1.d03 * m2.d31;
		double _d02 = m1.d00 * m2.d02 + m1.d01 * m2.d12 + m1.d02 * m2.d22 + m1.d03 * m2.d32;
		double _d03 = m1.d00 * m2.d03 + m1.d01 * m2.d13 + m1.d02 * m2.d23 + m1.d03 * m2.d33;

		double _d10 = m1.d10 * m2.d00 + m1.d11 * m2.d10 + m1.d12 * m2.d20 + m1.d13 * m2.d30;
		double _d11 = m1.d10 * m2.d01 + m1.d11 * m2.d11 + m1.d12 * m2.d21 + m1.d13 * m2.d31;
		double _d12 = m1.d10 * m2.d02 + m1.d11 * m2.d12 + m1.d12 * m2.d22 + m1.d13 * m2.d32;
		double _d13 = m1.d10 * m2.d03 + m1.d11 * m2.d13 + m1.d12 * m2.d23 + m1.d13 * m2.d33;

		double _d20 = m1.d20 * m2.d00 + m1.d21 * m2.d10 + m1.d22 * m2.d20 + m1.d23 * m2.d30;
		double _d21 = m1.d20 * m2.d01 + m1.d21 * m2.d11 + m1.d22 * m2.d21 + m1.d23 * m2.d31;
		double _d22 = m1.d20 * m2.d02 + m1.d21 * m2.d12 + m1.d22 * m2.d22 + m1.d23 * m2.d32;
		double _d23 = m1.d20 * m2.d03 + m1.d21 * m2.d13 + m1.d22 * m2.d23 + m1.d23 * m2.d33;

		double _d30 = m1.d30 * m2.d00 + m1.d31 * m2.d10 + m1.d32 * m2.d20 + m1.d33 * m2.d30;
		double _d31 = m1.d30 * m2.d01 + m1.d31 * m2.d11 + m1.d32 * m2.d21 + m1.d33 * m2.d31;
		double _d32 = m1.d30 * m2.d02 + m1.d31 * m2.d12 + m1.d32 * m2.d22 + m1.d33 * m2.d32;
		double _d33 = m1.d30 * m2.d03 + m1.d31 * m2.d13 + m1.d32 * m2.d23 + m1.d33 * m2.d33;

		d00 = _d00; d01 = _d01; d02 = _d02; d03 = _d03;
		d10 = _d10; d11 = _d11; d12 = _d12; d13 = _d13;
		d20 = _d20; d21 = _d21; d22 = _d22; d23 = _d23;
		d30 = _d30; d31 = _d31; d32 = _d32; d33 = _d33;
	}

	// --- rotation ---
	void set_rot3(const Quaternion& q);
	void set_rot4(const Quaternion& q);
	void set_euler3(const Vec3d& v);
	void set_euler4(const Vec3d& v);

	// --- translation ---
	void set_translate3(const Vec3d& v) noexcept {
		d03 = v.x; d13 = v.y; d23 = v.z;
	}

	// --- scale ---
	void scale2(double s) noexcept {
		d00 *= s; d01 *= s;
		d10 *= s; d11 *= s;
	}
	void scale3(double s) noexcept {
		d00 *= s; d01 *= s; d02 *= s;
		d10 *= s; d11 *= s; d12 *= s;
		d20 *= s; d21 *= s; d22 *= s;
	}
	void scale4(double s) noexcept {
		d00 *= s; d01 *= s; d02 *= s; d03 *= s;
		d10 *= s; d11 *= s; d12 *= s; d13 *= s;
		d20 *= s; d21 *= s; d22 *= s; d23 *= s;
		d30 *= s; d31 *= s; d32 *= s; d33 *= s;
	}

	void scale_rows2(const Vec2d& v) noexcept {
		d00 *= v.x; d01 *= v.x; d02 *= v.x; d03 *= v.x;
		d10 *= v.y; d11 *= v.y; d12 *= v.y; d13 *= v.y;
	}
	void scale_rows3(const Vec3d& v) noexcept {
		d00 *= v.x; d01 *= v.x; d02 *= v.x; d03 *= v.x;
		d10 *= v.y; d11 *= v.y; d12 *= v.y; d13 *= v.y;
		d20 *= v.z; d21 *= v.z; d22 *= v.z; d23 *= v.z;
	}
	void scale_rows4(const Vec4d& v) noexcept {
		d00 *= v.x; d01 *= v.x; d02 *= v.x; d03 *= v.x;
		d10 *= v.y; d11 *= v.y; d12 *= v.y; d13 *= v.y;
		d20 *= v.z; d21 *= v.z; d22 *= v.z; d23 *= v.z;
		d30 *= v.w; d31 *= v.w; d32 *= v.w; d33 *= v.w;
	}

	void scale_cols2(const Vec2d& v) noexcept {
		d00 *= v.x; d01 *= v.y;
		d10 *= v.x; d11 *= v.y;
		d20 *= v.x; d21 *= v.y;
		d30 *= v.x; d31 *= v.y;
	}
	void scale_cols3(const Vec3d& v) noexcept {
		d00 *= v.x; d01 *= v.y; d02 *= v.z;
		d10 *= v.x; d11 *= v.y; d12 *= v.z;
		d20 *= v.x; d21 *= v.y; d22 *= v.z;
		d30 *= v.x; d31 *= v.y; d32 *= v.z;
	}
	void scale_cols4(const Vec4d& v) noexcept {
		d00 *= v.x; d01 *= v.y; d02 *= v.z; d03 *= v.w;
		d10 *= v.x; d11 *= v.y; d12 *= v.z; d13 *= v.w;
		d20 *= v.x; d21 *= v.y; d22 *= v.z; d23 *= v.w;
		d30 *= v.x; d31 *= v.y; d32 *= v.z; d33 *= v.w;
	}

	// --- add ---
	void add4(const Mat4d& m) noexcept {
		d00 += m.d00; d01 += m.d01; d02 += m.d02; d03 += m.d03;
		d10 += m.d10; d11 += m.d11; d12 += m.d12; d13 += m.d13;
		d20 += m.d20; d21 += m.d21; d22 += m.d22; d23 += m.d23;
		d30 += m.d30; d31 += m.d31; d32 += m.d32; d33 += m.d33;
	}

	// --- determinant ---
	double determinant() const noexcept {
		double det = 0.0;
		if (d30 != 0.0) {
			det -= d30 * (d01 * d12 * d23 + d02 * d13 * d21 + d03 * d11 * d22
			            - d01 * d13 * d22 - d02 * d11 * d23 - d03 * d12 * d21);
		}
		if (d31 != 0.0) {
			det += d31 * (d00 * d12 * d23 + d02 * d13 * d20 + d03 * d10 * d22
			            - d00 * d13 * d22 - d02 * d10 * d23 - d03 * d12 * d20);
		}
		if (d32 != 0.0) {
			det -= d32 * (d00 * d11 * d23 + d01 * d13 * d20 + d03 * d10 * d21
			            - d00 * d13 * d21 - d01 * d10 * d23 - d03 * d11 * d20);
		}
		if (d33 != 0.0) {
			det += d33 * (d00 * d11 * d22 + d01 * d12 * d20 + d02 * d10 * d21
			            - d00 * d12 * d21 - d01 * d10 * d22 - d02 * d11 * d20);
		}
		return det;
	}

	// --- inverse ---
	Mat4d inverse() const {
		double det = determinant();
		if (det == 0.0) {
			throw std::runtime_error("Matrix is not invertible");
		}
		double inv_det = 1.0 / det;
		auto data = to_cm_data_array();
		std::array<double, 9> scratch{};

		Mat4d ret;
		ret.d00 = inv_det * cofactor(0, 0, data, scratch);
		ret.d01 = inv_det * cofactor(0, 1, data, scratch);
		ret.d02 = inv_det * cofactor(0, 2, data, scratch);
		ret.d03 = inv_det * cofactor(0, 3, data, scratch);

		ret.d10 = inv_det * cofactor(1, 0, data, scratch);
		ret.d11 = inv_det * cofactor(1, 1, data, scratch);
		ret.d12 = inv_det * cofactor(1, 2, data, scratch);
		ret.d13 = inv_det * cofactor(1, 3, data, scratch);

		ret.d20 = inv_det * cofactor(2, 0, data, scratch);
		ret.d21 = inv_det * cofactor(2, 1, data, scratch);
		ret.d22 = inv_det * cofactor(2, 2, data, scratch);
		ret.d23 = inv_det * cofactor(2, 3, data, scratch);

		ret.d30 = inv_det * cofactor(3, 0, data, scratch);
		ret.d31 = inv_det * cofactor(3, 1, data, scratch);
		ret.d32 = inv_det * cofactor(3, 2, data, scratch);
		ret.d33 = inv_det * cofactor(3, 3, data, scratch);

		return ret;
	}

	// --- column-major data array ---
	std::array<double, 16> to_cm_data_array() const noexcept {
		return {{
			d00, d10, d20, d30,
			d01, d11, d21, d31,
			d02, d12, d22, d32,
			d03, d13, d23, d33
		}};
	}

	// --- near-identity checks ---
	bool near_identity() const noexcept { return near_identity_thresh(0.0001); }
	bool near_identity_thresh(double threshold) const noexcept {
		bool ret = true;
		ret = ret && (std::abs(d00 - 1.0) < threshold);
		ret = ret && (std::abs(d11 - 1.0) < threshold);
		ret = ret && (std::abs(d22 - 1.0) < threshold);
		ret = ret && (std::abs(d33 - 1.0) < threshold);
		ret = ret && (std::abs(d01) < threshold);
		ret = ret && (std::abs(d02) < threshold);
		ret = ret && (std::abs(d03) < threshold);
		ret = ret && (std::abs(d10) < threshold);
		ret = ret && (std::abs(d12) < threshold);
		ret = ret && (std::abs(d13) < threshold);
		ret = ret && (std::abs(d20) < threshold);
		ret = ret && (std::abs(d21) < threshold);
		ret = ret && (std::abs(d23) < threshold);
		ret = ret && (std::abs(d30) < threshold);
		ret = ret && (std::abs(d31) < threshold);
		ret = ret && (std::abs(d32) < threshold);
		return ret;
	}
	bool near_identity3() const noexcept { return near_identity_thresh3(0.0001); }
	bool near_identity_thresh3(double threshold) const noexcept {
		bool ret = true;
		ret = ret && (std::abs(d00 - 1.0) < threshold);
		ret = ret && (std::abs(d11 - 1.0) < threshold);
		ret = ret && (std::abs(d22 - 1.0) < threshold);
		ret = ret && (std::abs(d01) < threshold);
		ret = ret && (std::abs(d02) < threshold);
		ret = ret && (std::abs(d10) < threshold);
		ret = ret && (std::abs(d12) < threshold);
		ret = ret && (std::abs(d20) < threshold);
		ret = ret && (std::abs(d21) < threshold);
		return ret;
	}

	bool near4(const Mat4d& m) const noexcept {
		return near(d00, m.d00) && near(d01, m.d01) && near(d02, m.d02) && near(d03, m.d03)
		    && near(d10, m.d10) && near(d11, m.d11) && near(d12, m.d12) && near(d13, m.d13)
		    && near(d20, m.d20) && near(d21, m.d21) && near(d22, m.d22) && near(d23, m.d23)
		    && near(d30, m.d30) && near(d31, m.d31) && near(d32, m.d32) && near(d33, m.d33);
	}

	// --- operator overloads ---
	friend Mat4d operator*(const Mat4d& a, const Mat4d& b) noexcept {
		Mat4d r;
		r.mult4(a, b);
		return r;
	}
	Mat4d& operator*=(const Mat4d& m) noexcept { mult4(m); return *this; }

	friend bool operator==(const Mat4d& a, const Mat4d& b) noexcept {
		return a.near4(b);
	}
	friend bool operator!=(const Mat4d& a, const Mat4d& b) noexcept {
		return !a.near4(b);
	}

private:
	static double cofactor(int x, int y, const std::array<double, 16>& data,
	                       std::array<double, 9>& sub) noexcept {
		int next_val = 0;
		for (int row = 0; row < 4; ++row) {
			if (row == x) continue;
			for (int col = 0; col < 4; ++col) {
				if (col == y) continue;
				sub[next_val++] = data[row * 4 + col];
			}
		}
		double ret = 0.0;
		ret += sub[0] * sub[4] * sub[8];
		ret += sub[1] * sub[5] * sub[6];
		ret += sub[2] * sub[3] * sub[7];
		ret -= sub[2] * sub[4] * sub[6];
		ret -= sub[1] * sub[3] * sub[8];
		ret -= sub[0] * sub[5] * sub[7];
		if ((x + y) % 2 != 0) {
			ret *= -1.0;
		}
		return ret;
	}
};

// ============================================================================
// Out-of-line definitions that need Quaternion
// ============================================================================

// Vec2d matrix multiplies (needs Mat4d)
inline void Vec2d::mult2(const Mat4d& m, const Vec2d& v) {
	x = m.d00 * v.x + m.d01 * v.y;
	y = m.d10 * v.x + m.d11 * v.y;
}
inline void Vec2d::mult2(const Vec2d& v, const Mat4d& m) {
	x = v.x * m.d00 + v.y * m.d10;
	y = v.x * m.d01 + v.y * m.d11;
}

// Vec3d matrix multiplies (needs Mat4d)
inline void Vec3d::mult3(const Mat4d& m, const Vec3d& v) {
	x = m.d00 * v.x + m.d01 * v.y + m.d02 * v.z;
	y = m.d10 * v.x + m.d11 * v.y + m.d12 * v.z;
	z = m.d20 * v.x + m.d21 * v.y + m.d22 * v.z;
}
inline void Vec3d::mult3(const Vec3d& v, const Mat4d& m) {
	x = v.x * m.d00 + v.y * m.d10 + v.z * m.d20;
	y = v.x * m.d01 + v.y * m.d11 + v.z * m.d21;
	z = v.x * m.d02 + v.y * m.d12 + v.z * m.d22;
}
inline void Vec3d::mult_and_trans3(const Mat4d& m, const Vec3d& v) {
	x = m.d00 * v.x + m.d01 * v.y + m.d02 * v.z + m.d03;
	y = m.d10 * v.x + m.d11 * v.y + m.d12 * v.z + m.d13;
	z = m.d20 * v.x + m.d21 * v.y + m.d22 * v.z + m.d23;
}

// Vec4d matrix multiplies (needs Mat4d)
inline void Vec4d::mult4(const Mat4d& m, const Vec4d& v) {
	x = m.d00 * v.x + m.d01 * v.y + m.d02 * v.z + m.d03 * v.w;
	y = m.d10 * v.x + m.d11 * v.y + m.d12 * v.z + m.d13 * v.w;
	z = m.d20 * v.x + m.d21 * v.y + m.d22 * v.z + m.d23 * v.w;
	w = m.d30 * v.x + m.d31 * v.y + m.d32 * v.z + m.d33 * v.w;
}
inline void Vec4d::mult4(const Vec4d& v, const Mat4d& m) {
	x = v.x * m.d00 + v.y * m.d10 + v.z * m.d20 + v.w * m.d30;
	y = v.x * m.d01 + v.y * m.d11 + v.z * m.d21 + v.w * m.d31;
	z = v.x * m.d02 + v.y * m.d12 + v.z * m.d22 + v.w * m.d32;
	w = v.x * m.d03 + v.y * m.d13 + v.z * m.d23 + v.w * m.d33;
}

// ============================================================================
// Rotation methods (need Quaternion definition)
// ============================================================================
#include "quaternion.h"

inline void Mat4d::set_rot3(const Quaternion& q) {
	double xsq = q.x * q.x;
	double ysq = q.y * q.y;
	double zsq = q.z * q.z;
	double wsq = q.w * q.w;

	d00 = wsq + xsq - ysq - zsq;
	d01 = 2.0 * (q.y * q.x - q.w * q.z);
	d02 = 2.0 * (q.z * q.x + q.w * q.y);

	d10 = 2.0 * (q.x * q.y + q.w * q.z);
	d11 = wsq - xsq + ysq - zsq;
	d12 = 2.0 * (q.z * q.y - q.w * q.x);

	d20 = 2.0 * (q.x * q.z - q.w * q.y);
	d21 = 2.0 * (q.y * q.z + q.w * q.x);
	d22 = wsq - xsq - ysq + zsq;
}

inline void Mat4d::set_rot4(const Quaternion& q) {
	set_rot3(q);
	d03 = 0.0; d13 = 0.0; d23 = 0.0;
	d30 = 0.0; d31 = 0.0; d32 = 0.0;
	d33 = 1.0;
}

inline void Mat4d::set_euler3(const Vec3d& v) {
	double sinx = std::sin(v.x);
	double siny = std::sin(v.y);
	double sinz = std::sin(v.z);
	double cosx = std::cos(v.x);
	double cosy = std::cos(v.y);
	double cosz = std::cos(v.z);

	d00 = cosy * cosz;
	d01 = -(cosx * sinz) + (sinx * siny * cosz);
	d02 = (sinx * sinz) + (cosx * siny * cosz);

	d10 = cosy * sinz;
	d11 = (cosx * cosz) + (sinx * siny * sinz);
	d12 = -(sinx * cosz) + (cosx * siny * sinz);

	d20 = -siny;
	d21 = sinx * cosy;
	d22 = cosx * cosy;
}

inline void Mat4d::set_euler4(const Vec3d& v) {
	set_euler3(v);
	d03 = 0.0; d13 = 0.0; d23 = 0.0;
	d30 = 0.0; d31 = 0.0; d32 = 0.0;
	d33 = 1.0;
}

}  // namespace jaamsim::math

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

#include <string>

#include "math_utils.h"
#include "mat4d.h"
#include "quaternion.h"
#include "vec3d.h"
#include "vec4d.h"

namespace jaamsim::math {

class Transform {
public:
	static const Transform ident;

	Transform()
		: _trans{}, _rot{}, _scale{1.0}, _matrix_dirty{false} {}

	explicit Transform(const Transform& t)
		: _trans{t._trans}, _rot{t._rot}, _scale{t._scale}, _matrix_dirty{true}
	{
		if (!t._matrix_dirty) {
			_mat4d.set4(t._mat4d);
			_matrix_dirty = false;
		}
	}

	Transform(const Vec3d& trans, const Quaternion& rot, double scale)
		: _trans{trans}, _rot{rot}, _scale{scale}, _matrix_dirty{true} {}

	explicit Transform(const Vec3d& trans)
		: Transform{trans, Quaternion{}, 1.0} {}

	void copy_from(const Transform& t) {
		_trans.set3(t._trans);
		_rot.set(t._rot);
		_scale = t._scale;
		if (!t._matrix_dirty) {
			_mat4d.set4(t._mat4d);
		}
		_matrix_dirty = t._matrix_dirty;
	}

	void set_trans(const Vec3d& trans) {
		if (_trans.equals3(trans)) return;
		_trans.set3(trans);
		_matrix_dirty = true;
	}

	void set_rot(const Quaternion& q) {
		if (_rot.equals(q)) return;
		_rot.set(q);
		_matrix_dirty = true;
	}

	void set_scale(double s) {
		if (near(_scale, s)) return;
		_scale = s;
		_matrix_dirty = true;
	}

	void get_rot(Quaternion& out) const { out.set(_rot); }
	const Quaternion& get_rot_ref() const { return _rot; }
	const Vec3d& get_trans_ref() const { return _trans; }
	double get_scale() const { return _scale; }

	void get_mat4d(Mat4d& out) {
		if (_matrix_dirty) update_matrix();
		out.set4(_mat4d);
	}

	const Mat4d& get_mat4d_ref() {
		if (_matrix_dirty) update_matrix();
		return _mat4d;
	}

	void merge(const Transform& a, const Transform& b) {
		Vec3d temp{a._trans};
		Mat4d rot_temp;
		rot_temp.set_rot3(a._rot);
		_trans.mult3(rot_temp, b._trans);
		_trans.scale3(a._scale);
		_trans.add3(temp);
		_matrix_dirty = true;
		_rot.mult(a._rot, b._rot);
		_scale = a._scale * b._scale;
	}

	void apply(const Vec4d& vect, Vec4d& out) {
		if (_matrix_dirty) update_matrix();
		out.mult4(_mat4d, vect);
	}

	void mult_and_trans(const Vec3d& vect, Vec3d& out) {
		if (_matrix_dirty) update_matrix();
		out.mult_and_trans3(_mat4d, vect);
	}

	void inverse(Transform& out) const {
		out._scale = 1.0 / _scale;
		out._rot.conjugate(_rot);
		out._trans.set3(_trans);
		out._trans.scale3(-out._scale);
		Mat4d rot_temp;
		rot_temp.set_rot3(out._rot);
		out._trans.mult3(rot_temp, out._trans);
		out._matrix_dirty = true;
	}

	bool equals(const Transform& t) const noexcept {
		return _trans.equals3(t._trans) && _rot.equals(t._rot) && near(_scale, t._scale);
	}
	bool near(const Transform& t) const noexcept {
		return _trans.near3(t._trans) && _rot.near(t._rot) && math::near(_scale, t._scale);
	}

	std::string to_string() const {
		return "T: " + _trans.to_string() + " R: " + _rot.to_string()
		     + " S: " + std::to_string(_scale);
	}

private:
	void update_matrix() {
		_mat4d.set_rot4(_rot);
		_mat4d.set_translate3(_trans);
		_mat4d.scale3(_scale);
		_matrix_dirty = false;
	}

	Quaternion _rot;
	Vec3d _trans;
	double _scale;
	Mat4d _mat4d;
	bool _matrix_dirty;
};

inline const Transform Transform::ident{};

}  // namespace jaamsim::math

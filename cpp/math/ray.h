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

#include "vec4d.h"

namespace jaamsim::math {

struct Transform;
struct Mat4d;

class Ray {
public:
	Ray() : _start{0.0, 0.0, 0.0, 1.0}, _direction{1.0, 0.0, 0.0, 0.0} {}

	Ray(const Vec4d& start, const Vec4d& dir)
		: _start{start}, _direction{dir}
	{
		_direction.normalize3();
		_direction.w = 0.0;
	}

	const Vec4d& get_start_ref() const { return _start; }
	const Vec4d& get_dir_ref() const { return _direction; }

	Ray transform(const Transform& trans) const;
	Ray transform(const Mat4d& mat) const;

	Vec3d get_point_at_dist(double dist) const {
		Vec3d ret{_direction};
		ret.scale3(dist);
		ret.add3(_start);
		return ret;
	}

	double get_dist_along_ray(const Vec3d& point) const {
		Vec3d diff{point};
		diff.sub3(_start);
		return diff.dot3(_direction);
	}

	std::string to_string() const {
		return "Orig: " + _start.to_string() + " Dir: " + _direction.to_string();
	}

private:
	Vec4d _start;
	Vec4d _direction;
};

// ============================================================================
// Out-of-line definitions that need Transform and Mat4d
// ============================================================================
#include "transform.h"
#include "mat4d.h"

inline Ray Ray::transform(const Transform& trans) const {
	return transform(trans.get_mat4d_ref());
}

inline Ray Ray::transform(const Mat4d& mat) const {
	Vec4d start_transed{0.0, 0.0, 0.0, 1.0};
	start_transed.mult4(mat, _start);
	Vec4d dir_transed{0.0, 0.0, 0.0, 1.0};
	dir_transed.mult4(mat, _direction);
	dir_transed.normalize3();
	return Ray{start_transed, dir_transed};
}

}  // namespace jaamsim::math

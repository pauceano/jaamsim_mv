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
#include <limits>

#include "math_utils.h"
#include "mat4d.h"
#include "transform.h"
#include "vec3d.h"

namespace jaamsim::math {

struct Ray;      // forward declaration
struct Vec4d;

class Plane {
public:
	Vec3d normal;
	double dist() const { return _dist; }

	Plane() : normal{0.0, 0.0, 1.0}, _dist{0.0} {}
	Plane(const Vec3d& norm, double distance) {
		set(norm, distance);
	}
	Plane(const Vec3d& p0, const Vec3d& p1, const Vec3d& p2) {
		set(p0, p1, p2);
	}

	void set(const Vec3d& norm, double distance) {
		normal.normalize3(norm);
		_dist = distance;
	}

	void set(const Vec3d& p0, const Vec3d& p1, const Vec3d& p2) {
		Vec3d v0;
		v0.sub3(p1, p0);
		Vec3d v1;
		v1.sub3(p2, p1);
		normal.cross3(v0, v1);
		normal.normalize3();
		_dist = normal.dot3(p0);
	}

	double get_normal_dist(const Vec3d& point) const noexcept {
		return point.dot3(normal) - _dist;
	}

	void transform(const Transform& t, const Plane& p) {
		transform(t.get_mat4d_ref(), t.get_mat4d_ref(), p);
	}

	void transform(const Mat4d& mat, const Mat4d& normal_mat, const Plane& p) {
		Vec3d close_point;
		close_point.scale3(p._dist, p.normal);
		close_point.mult_and_trans3(mat, close_point);
		normal.mult3(normal_mat, p.normal);
		normal.normalize3();
		_dist = normal.dot3(close_point);
	}

	bool near(const Plane& p) const noexcept {
		return normal.near3(p.normal) && math::near(_dist, p._dist);
	}

	bool equals(const Plane& p) const noexcept {
		return normal.equals3(p.normal) && math::near(_dist, p._dist);
	}

	double collision_dist(const Ray& r) const;
	Ray collide(const Plane& p) const;
	bool back_face_collision(const Ray& r) const;

private:
	double _dist;
};

// ============================================================================
// Out-of-line definitions that need Ray
// ============================================================================
#include "ray.h"

inline double Plane::collision_dist(const Ray& r) const {
	double cos_val = -1.0 * normal.dot3(r.get_dir_ref());
	if (near(cos_val, 0.0)) {
		return std::numeric_limits<double>::infinity();
	}
	return (normal.dot3(r.get_start_ref()) - _dist) / cos_val;
}

inline bool Plane::back_face_collision(const Ray& r) const {
	return normal.dot3(r.get_dir_ref()) > 0.0;
}

inline Ray Plane::collide(const Plane& p) const {
	double norm_dot = normal.dot3(p.normal);
	if (near(norm_dot, 1.0) || near(norm_dot, -1.0)) {
		throw std::runtime_error("Planes are parallel, no intersection ray");
	}

	Vec3d ray_dir;
	ray_dir.cross3(normal, p.normal);
	ray_dir.normalize3();

	Vec3d int_dir;
	int_dir.cross3(ray_dir, normal);
	int_dir.normalize3();

	Vec3d int_start{normal};
	int_start.scale3(_dist);

	Ray intersect_ray{Vec4d{int_start, 1.0}, Vec4d{int_dir, 0.0}};
	double int_dist = p.collision_dist(intersect_ray);

	Vec3d int_point{int_dir};
	int_point.scale3(int_dist);
	int_point.add3(int_start);

	return Ray{Vec4d{int_point, 1.0}, Vec4d{ray_dir, 0.0}};
}

}  // namespace jaamsim::math

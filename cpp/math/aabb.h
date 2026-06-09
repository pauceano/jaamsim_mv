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
#include <vector>

#include "math_utils.h"
#include "plane.h"
#include "vec3d.h"
#include "vec4d.h"

namespace jaamsim::math {

class AABB {
public:
	enum class PlaneTestResult { collides, positive, negative, empty };

	Vec3d max_pt;
	Vec3d min_pt;
	Vec3d center;
	Vec3d radius;

	AABB() : _is_empty{true} {}

	explicit AABB(const AABB& other)
		: _is_empty{other._is_empty}
	{
		min_pt.set3(other.min_pt);
		max_pt.set3(other.max_pt);
		update_center_and_radius();
	}

	AABB(const Vec3d& pos_point, const Vec3d& neg_point)
	{
		max_pt.set3(pos_point);
		min_pt.set3(neg_point);
		update_center_and_radius();
	}

	explicit AABB(const std::vector<Vec3d>& points, double fudge)
		: AABB{points}
	{
		max_pt.x += fudge; max_pt.y += fudge; max_pt.z += fudge;
		min_pt.x -= fudge; min_pt.y -= fudge; min_pt.z -= fudge;
		update_center_and_radius();
	}

	explicit AABB(const std::vector<Vec3d>& points) {
		if (points.empty()) {
			_is_empty = true;
			return;
		}
		max_pt.set3(points[0]);
		min_pt.set3(points[0]);
		for (const auto& p : points) {
			max_pt.max3(p);
			min_pt.min3(p);
		}
		update_center_and_radius();
	}

	AABB(const std::vector<Vec3d>& points, const Mat4d& trans) {
		if (points.empty()) {
			_is_empty = true;
			return;
		}
		Vec3d p;
		p.mult_and_trans3(trans, points[0]);
		max_pt.set3(p);
		min_pt.set3(p);
		for (const auto& p_orig : points) {
			p.mult_and_trans3(trans, p_orig);
			max_pt.max3(p);
			min_pt.min3(p);
		}
		update_center_and_radius();
	}

	bool collides(const Vec3d& point, double fudge = 0.0) const noexcept {
		if (_is_empty) return false;
		bool bx = point.x > min_pt.x - fudge && point.x < max_pt.x + fudge;
		bool by = point.y > min_pt.y - fudge && point.y < max_pt.y + fudge;
		bool bz = point.z > min_pt.z - fudge && point.z < max_pt.z + fudge;
		return bx && by && bz;
	}

	bool collides(const AABB& other, double fudge = 0.0) const noexcept {
		if (_is_empty || other._is_empty) return false;
		bool bx = seg_overlap(min_pt.x, max_pt.x, other.min_pt.x, other.max_pt.x, fudge);
		bool by = seg_overlap(min_pt.y, max_pt.y, other.min_pt.y, other.max_pt.y, fudge);
		bool bz = seg_overlap(min_pt.z, max_pt.z, other.min_pt.z, other.max_pt.z, fudge);
		return bx && by && bz;
	}

	double collision_dist(const Ray& r, double fudge = 0.0) const;

	bool is_empty() const noexcept { return _is_empty; }

	PlaneTestResult test_to_plane(const Plane& p) const {
		if (_is_empty) return PlaneTestResult::empty;

		double effective_radius = 0.0;
		effective_radius += radius.x * std::abs(p.normal.x);
		effective_radius += radius.y * std::abs(p.normal.y);
		effective_radius += radius.z * std::abs(p.normal.z);

		double center_dist = p.get_normal_dist(center);
		if (center_dist > effective_radius) return PlaneTestResult::positive;
		if (center_dist < -effective_radius) return PlaneTestResult::negative;
		return PlaneTestResult::collides;
	}

private:
	void update_center_and_radius() {
		center.add3(max_pt, min_pt);
		center.scale3(0.5);
		radius.sub3(max_pt, min_pt);
		radius.scale3(0.5);
	}

	bool _is_empty;
};

// ============================================================================
// Out-of-line: AABB::collision_dist (complex, uses Plane and Ray)
// ============================================================================

inline double AABB::collision_dist(const Ray& r, double fudge) const {
	if (_is_empty) return -1.0;

	const auto& rs = r.get_start_ref();
	if (collides(Vec3d{rs.x, rs.y, rs.z}, fudge)) {
		return 0.0;
	}

	const auto& ray_dir = r.get_dir_ref();
	for (int axis = 0; axis < 3; ++axis) {
		double dir_comp = 0.0;
		if (axis == 0) dir_comp = ray_dir.x;
		else if (axis == 1) dir_comp = ray_dir.y;
		else dir_comp = ray_dir.z;

		if (near(dir_comp, 0.0)) continue;

		Vec4d face_norm{0.0, 0.0, 0.0, 1.0};
		double face_dist = 0.0;

		double min_comp = (axis == 0) ? min_pt.x : (axis == 1) ? min_pt.y : min_pt.z;
		double max_comp = (axis == 0) ? max_pt.x : (axis == 1) ? max_pt.y : max_pt.z;

		if (dir_comp > 0.0) {
			if (axis == 0) face_norm.x = -1.0;
			else if (axis == 1) face_norm.y = -1.0;
			else face_norm.z = -1.0;
			face_dist = -min_comp - fudge;
		} else {
			if (axis == 0) face_norm.x = 1.0;
			else if (axis == 1) face_norm.y = 1.0;
			else face_norm.z = 1.0;
			face_dist = max_comp + fudge;
		}

		Plane face_plane{Vec3d{face_norm.x, face_norm.y, face_norm.z}, face_dist};
		double ray_collision_dist = face_plane.collision_dist(r);

		if (std::isinf(ray_collision_dist)) continue;
		if (ray_collision_dist < 0.0) continue;

		int a1 = (axis + 1) % 3;
		int a2 = (axis + 2) % 3;

		Vec3d contact_point = r.get_point_at_dist(ray_collision_dist);

		auto get_comp = [](const Vec3d& v, int i) -> double {
			if (i == 0) return v.x;
			if (i == 1) return v.y;
			return v.z;
		};

		auto get_min = [&](int i) -> double {
			if (i == 0) return min_pt.x;
			if (i == 1) return min_pt.y;
			return min_pt.z;
		};

		auto get_max = [&](int i) -> double {
			if (i == 0) return max_pt.x;
			if (i == 1) return max_pt.y;
			return max_pt.z;
		};

		if (get_comp(contact_point, a1) < get_min(a1) - fudge
		 || get_comp(contact_point, a1) > get_max(a1) + fudge) continue;
		if (get_comp(contact_point, a2) < get_min(a2) - fudge
		 || get_comp(contact_point, a2) > get_max(a2) + fudge) continue;

		return ray_collision_dist;
	}
	return -1.0;
}

}  // namespace jaamsim::math

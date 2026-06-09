/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2012 Ausenco Engineering Canada Inc.
 * Copyright (C) 2019-2026 JaamSim Software Inc.
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
#include <cstddef>

namespace jaamsim::math {

constexpr double EPSILON = 0.000000001;  // one billionth

/**
 * Test whether a double value is zero within a tolerance.
 */
constexpr bool is_small(double a) noexcept {
	return a < EPSILON;
}

/**
 * Compares two doubles for equality within a tolerance.
 */
constexpr bool near(double a, double b) noexcept {
	double diff = std::abs(a - b);
	return is_small(diff);
}

/**
 * Performs a greater-than-or-equal-to comparison within a tolerance.
 */
constexpr bool near_gt(double a, double b) noexcept {
	return a + EPSILON > b;
}

/**
 * Performs a less-than-or-equal-to comparison within a tolerance.
 */
constexpr bool near_lt(double a, double b) noexcept {
	return a < b + EPSILON;
}

/**
 * Performs a strict greater-than comparison within a tolerance.
 */
constexpr bool strict_gt(double a, double b) noexcept {
	return !near_lt(a, b);
}

/**
 * Performs a strict less-than comparison within a tolerance.
 */
constexpr bool strict_lt(double a, double b) noexcept {
	return !near_gt(a, b);
}

/**
 * Checks for line segment overlap.
 */
constexpr bool seg_overlap(double a0, double a1, double b0, double b1) noexcept {
	if (a0 == b0) return true;
	if (a0 < b0) {
		return b0 <= a1;
	}
	return a0 <= b1;
}

/**
 * Checks for line segment overlap with a fudge factor.
 */
constexpr bool seg_overlap(double a0, double a1, double b0, double b1, double fudge) noexcept {
	if (a0 == b0) return true;
	if (a0 < b0) {
		return b0 <= a1 + fudge;
	}
	return a0 <= b1 + fudge;
}

/**
 * Perform a bounds check on val, returns something in the range [min, max].
 */
constexpr double bound(double val, double min, double max) noexcept {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

/**
 * Returns whether the given magnitude can be used to normalize a Vec.
 */
constexpr bool non_normal_mag(double mag) noexcept {
	return mag == 0.0 || std::isnan(mag) || std::isinf(mag);
}

}  // namespace jaamsim::math

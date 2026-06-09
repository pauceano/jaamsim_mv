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

#include <cstddef>
#include <string>

namespace jaamsim::math {

class TessFontKey {
public:
	enum class FontStyle { plain = 0 };

	TessFontKey() : _font_name{}, _font_style{0} {}
	explicit TessFontKey(const std::string& font_name)
		: _font_name{font_name}, _font_style{0} {}
	TessFontKey(const std::string& font_name, int font_style)
		: _font_name{font_name}, _font_style{font_style} {}

	const std::string& get_font_name() const { return _font_name; }
	int get_font_style() const { return _font_style; }

	bool equals(const TessFontKey& other) const noexcept {
		return _font_name == other._font_name && _font_style == other._font_style;
	}

	friend bool operator==(const TessFontKey& a, const TessFontKey& b) noexcept {
		return a.equals(b);
	}
	friend bool operator!=(const TessFontKey& a, const TessFontKey& b) noexcept {
		return !a.equals(b);
	}

private:
	std::string _font_name;
	int _font_style;
};

}  // namespace jaamsim::math

namespace std {
	template <>
	struct hash<jaamsim::math::TessFontKey> {
		std::size_t operator()(const jaamsim::math::TessFontKey& k) const noexcept {
			std::size_t code = std::hash<std::string>{}(k.get_font_name());
			code ^= static_cast<std::size_t>(k.get_font_style()) * 71;
			return code;
		}
	};
}

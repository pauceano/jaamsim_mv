/*
 * JaamSim Discrete Event Simulation
 * Copyright (C) 2022-2026 JaamSim Software Inc.
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

#include <cstdio>
#include <string>
#include <vector>

#include "json_value.h"

namespace jaamsim::json {

namespace detail {

inline bool must_escape_char(char c) {
    if (static_cast<unsigned char>(c) <= 0x1F) return true;
    return c == '\\' || c == '"';
}

inline std::string escape_seq(char c) {
    switch (c) {
    case '\n': return "\\n";
    case '\r': return "\\r";
    case '\t': return "\\t";
    case '\b': return "\\b";
    case '\f': return "\\f";
    case '"':  return "\\\"";
    case '\\': return "\\\\";
    default:   break;
    }
    char buf[8];
    std::snprintf(buf, sizeof(buf), "\\u%04x",
        static_cast<unsigned char>(c));
    return buf;
}

inline std::string escape_string(const std::string& s) {
    std::string out;
    for (const char c : s) {
        if (must_escape_char(c)) {
            out += escape_seq(c);
        } else {
            out += c;
        }
    }
    return out;
}

inline void write_val(const Value& val, std::string& out);

inline void write_list(const std::vector<Value>& arr, std::string& out) {
    out += '[';
    const std::size_t n = arr.size();
    for (std::size_t i = 0; i < n; i++) {
        write_val(arr[i], out);
        if (i + 1 < n) {
            out += ", ";
        }
    }
    out += ']';
}

inline void write_map(
    const std::unordered_map<std::string, Value>& obj, std::string& out) {
    out += '{';
    std::size_t i = 0;
    const std::size_t n = obj.size();
    for (const auto& [key, val] : obj) {
        out += '"';
        out += escape_string(key);
        out += "\": ";
        write_val(val, out);
        if (++i < n) {
            out += ", ";
        }
    }
    out += '}';
}

inline void write_val(const Value& val, std::string& out) {
    if (val.is_null()) {
        out += "null";
        return;
    }
    if (val.is_bool()) {
        out += val.is_true() ? "true" : "false";
        return;
    }
    if (val.is_number()) {
        char buf[32];
        const int written = std::snprintf(buf, sizeof(buf), "%.17g", val.num_val);
        if (written > 0) {
            out.append(buf, static_cast<std::size_t>(written));
        }
        return;
    }
    if (val.is_string()) {
        out += '"';
        out += escape_string(val.str_val);
        out += '"';
        return;
    }
    if (val.is_array()) {
        write_list(val.arr_val, out);
        return;
    }
    if (val.is_object()) {
        write_map(val.obj_val, out);
        return;
    }
}

}  // namespace detail

inline std::string to_json(const Value& val) {
    std::string out;
    detail::write_val(val, out);
    return out;
}

}  // namespace jaamsim::json

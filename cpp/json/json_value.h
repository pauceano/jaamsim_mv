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

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace jaamsim::json {

enum class Type { null_type, bool_type, number, string, array, object };

struct JsonError : std::runtime_error {
    std::string source;
    int pos;

    JsonError(std::string src, int p, const std::string& msg)
        : std::runtime_error(msg), source(std::move(src)), pos(p) {}
};

struct Value {
    Type type = Type::null_type;

    double num_val = 0.0;
    bool   bool_val = false;
    std::string str_val;
    std::vector<Value> arr_val;
    std::unordered_map<std::string, Value> obj_val;

    bool is_null()   const noexcept { return type == Type::null_type; }
    bool is_string() const noexcept { return type == Type::string; }
    bool is_number() const noexcept { return type == Type::number; }
    bool is_array()  const noexcept { return type == Type::array; }
    bool is_object() const noexcept { return type == Type::object; }
    bool is_bool()   const noexcept { return type == Type::bool_type; }
    bool is_true()   const noexcept { return is_bool() && bool_val; }
    bool is_false()  const noexcept { return is_bool() && !bool_val; }

    static Value make_string(std::string s) {
        Value v;
        v.type = Type::string;
        v.str_val = std::move(s);
        return v;
    }

    static Value make_number(double n) {
        Value v;
        v.type = Type::number;
        v.num_val = n;
        return v;
    }

    static Value make_bool(bool b) {
        Value v;
        v.type = Type::bool_type;
        v.bool_val = b;
        return v;
    }

    static Value make_null() {
        Value v;
        v.type = Type::null_type;
        return v;
    }

    static Value make_object() {
        Value v;
        v.type = Type::object;
        return v;
    }

    static Value make_array() {
        Value v;
        v.type = Type::array;
        return v;
    }
};

}  // namespace jaamsim::json

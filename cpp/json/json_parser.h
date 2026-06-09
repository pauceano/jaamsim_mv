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

#include <cassert>
#include <cstdlib>
#include <string>
#include <vector>

#include "json_tokenizer.h"

namespace jaamsim::json {

namespace detail {

inline bool is_sym(const Token& tok, const char* expected) {
    return tok.type == SYM_TYPE && tok.value == expected;
}

inline bool is_string_tok(const Token& tok) {
    return tok.type == STRING_TYPE;
}

inline int parse_element(
    const std::vector<Token>& toks, int start_pos, Value& out_val);

inline int parse_map(
    const std::vector<Token>& toks, int start_pos, Value& out_val);

inline int parse_list(
    const std::vector<Token>& toks, int start_pos, Value& out_val);

}  // namespace detail

inline Value parse_tokens(const std::vector<Token>& toks) {
    Value ret;
    detail::parse_element(toks, 0, ret);
    return ret;
}

inline Value parse(const std::string& json) {
    return parse_tokens(tokenize(json));
}

class IncrementalParser {
public:
    void add_piece(std::string piece) { pieces_.push_back(std::move(piece)); }

    bool scanning_error() const noexcept { return scanner_error_; }

    bool is_object() const noexcept { return top_elem_is_obj_; }

    bool is_element_complete() {
        if (scanner_error_) return false;
        if (top_elem_is_complete_) return true;

        std::size_t piece_pos = 0;

        while (true) {
            if (scanner_piece_ >= pieces_.size()) break;

            const std::string& cur_piece = pieces_[scanner_piece_];
            if (piece_pos >= cur_piece.size()) {
                piece_pos = 0;
                scanner_piece_++;
                continue;
            }

            const char scanned_char = cur_piece[piece_pos];
            piece_pos++;

            if (scanner_escaping_) {
                scanner_escaping_ = false;
                continue;
            }

            if (scanner_in_string_) {
                if (scanned_char == '\\') {
                    scanner_escaping_ = true;
                    continue;
                }
                if (scanned_char == '"') {
                    scanner_in_string_ = false;
                    continue;
                }
                continue;
            }

            if (scanned_char == '"') {
                scanner_in_string_ = true;
                continue;
            }

            if (scanned_char == '{') {
                if (!first_elem_found_) {
                    first_elem_found_ = true;
                    top_elem_is_obj_ = true;
                }
                obj_depth_++;
                continue;
            }
            if (scanned_char == '}') {
                if (obj_depth_ <= 0) {
                    scanner_error_ = true;
                    return false;
                }
                obj_depth_--;
                continue;
            }

            if (scanned_char == '[') {
                if (!first_elem_found_) {
                    first_elem_found_ = true;
                    top_elem_is_obj_ = false;
                }
                array_depth_++;
                continue;
            }
            if (scanned_char == ']') {
                if (array_depth_ <= 0) {
                    scanner_error_ = true;
                    return false;
                }
                array_depth_--;
                continue;
            }
        }

        if (obj_depth_ == 0 && array_depth_ == 0 && first_elem_found_) {
            top_elem_is_complete_ = true;
            return true;
        }
        return false;
    }

    Value parse() {
        std::string source;
        for (const auto& s : pieces_) {
            source += s;
        }

        const bool is_comp = is_element_complete();
        if (scanner_error_) {
            throw JsonError(source, -1, "Mismatched brackets detected");
        }
        if (!is_comp) {
            throw JsonError(source,
                static_cast<int>(source.size()), "Incomplete JSON element");
        }
        return parse_tokens(tokenize(source));
    }

private:
    std::vector<std::string> pieces_;
    std::size_t scanner_piece_ = 0;
    bool scanner_in_string_ = false;
    bool scanner_escaping_ = false;
    bool first_elem_found_ = false;
    bool top_elem_is_obj_ = false;
    bool scanner_error_ = false;
    int obj_depth_ = 0;
    int array_depth_ = 0;
    bool top_elem_is_complete_ = false;
};

namespace detail {

inline int parse_element(
    const std::vector<Token>& toks, int start_pos, Value& out_val) {
    int pos = start_pos;
    const Token& start_tok = toks[static_cast<std::size_t>(pos++)];

    switch (start_tok.type) {
    case NUM_TYPE: {
        const double n = std::strtod(start_tok.value.c_str(), nullptr);
        out_val = Value::make_number(n);
        return pos;
    }
    case STRING_TYPE:
        out_val = Value::make_string(start_tok.value);
        return pos;
    case SYM_TYPE:
        if (start_tok.value == "[") {
            return parse_list(toks, pos, out_val);
        }
        if (start_tok.value == "{") {
            return parse_map(toks, pos, out_val);
        }
        throw JsonError("", start_tok.pos, "Unexpected token to start element");
    case KEYWORD_TYPE:
        if (start_tok.value == "true") {
            out_val = Value::make_bool(true);
        } else if (start_tok.value == "false") {
            out_val = Value::make_bool(false);
        } else if (start_tok.value == "null") {
            out_val = Value::make_null();
        }
        return pos;
    default:
        throw JsonError("", start_tok.pos, "Internal error: Unknown token type");
    }
}

inline int parse_map(
    const std::vector<Token>& toks, int start_pos, Value& out_val) {
    int pos = start_pos;
    Token tok = toks[static_cast<std::size_t>(pos)];

    out_val = Value::make_object();

    if (is_sym(tok, "}")) {
        return pos + 1;
    }

    const int size = static_cast<int>(toks.size());
    while (pos < size) {
        if (!is_string_tok(tok)) {
            throw JsonError("", tok.pos, "Map keys must be strings");
        }
        const std::string key = tok.value;
        pos++;

        tok = toks[static_cast<std::size_t>(pos)];
        if (!is_sym(tok, ":")) {
            throw JsonError("", tok.pos, "Expected \":\"");
        }
        pos++;

        Value val;
        pos = parse_element(toks, pos, val);
        out_val.obj_val[std::move(key)] = std::move(val);

        tok = toks[static_cast<std::size_t>(pos)];

        if (is_sym(tok, "}")) {
            return pos + 1;
        }
        if (is_sym(tok, ",")) {
            pos++;
            tok = toks[static_cast<std::size_t>(pos)];
            continue;
        }
        throw JsonError("", tok.pos, "Unexpected symbol in map");
    }
    throw JsonError("", toks[static_cast<std::size_t>(start_pos)].pos,
        "Unterminated map");
}

inline int parse_list(
    const std::vector<Token>& toks, int start_pos, Value& out_val) {
    int pos = start_pos;
    Token tok = toks[static_cast<std::size_t>(pos)];

    out_val = Value::make_array();

    if (is_sym(tok, "]")) {
        return pos + 1;
    }

    const int size = static_cast<int>(toks.size());
    while (pos < size) {
        Value val;
        pos = parse_element(toks, pos, val);
        out_val.arr_val.push_back(std::move(val));

        tok = toks[static_cast<std::size_t>(pos)];

        if (is_sym(tok, "]")) {
            return pos + 1;
        }
        if (is_sym(tok, ",")) {
            pos++;
            continue;
        }
        throw JsonError("", tok.pos, "Unexpected symbol in list");
    }
    throw JsonError("", toks[static_cast<std::size_t>(start_pos)].pos,
        "Unterminated list");
}

}  // namespace detail

}  // namespace jaamsim::json

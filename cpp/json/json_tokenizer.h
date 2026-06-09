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
#include <cctype>
#include <charconv>
#include <cstdint>
#include <string>
#include <vector>

#include "json_value.h"

namespace jaamsim::json {

enum TokenType { STRING_TYPE = 0, NUM_TYPE = 1, SYM_TYPE = 2, KEYWORD_TYPE = 3 };

struct Token {
    int type = -1;
    std::string value;
    int pos = 0;
};

inline Token sym_tok(char c, int pos) {
    Token tok;
    tok.type = SYM_TYPE;
    tok.value = std::string(1, c);
    tok.pos = pos;
    return tok;
}

inline bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline bool is_number_start(char c) {
    return (c >= '0' && c <= '9') || c == '-';
}

inline char next_char(const std::string& input, int pos) {
    if (static_cast<std::size_t>(pos) >= input.size()) {
        return '$';
    }
    return input[static_cast<std::size_t>(pos)];
}

inline void append_utf8(std::string& s, uint32_t codepoint) {
    if (codepoint <= 0x7F) {
        s += static_cast<char>(codepoint);
    } else if (codepoint <= 0x7FF) {
        s += static_cast<char>(0xC0 | (codepoint >> 6));
        s += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        s += static_cast<char>(0xE0 | (codepoint >> 12));
        s += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        s += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else {
        s += static_cast<char>(0xF0 | (codepoint >> 18));
        s += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        s += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        s += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
}

namespace detail {

inline int get_string_token(
    std::vector<Token>& res, int start_pos, const std::string& input);

inline std::pair<std::string, int> get_digits(
    int start_pos, const std::string& input);

inline int get_num_token(
    std::vector<Token>& res, int start_pos, const std::string& input);

inline int get_keyword_token(
    std::vector<Token>& res, int pos, const std::string& input);

}  // namespace detail

inline std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> res;
    int i = 0;
    const int len = static_cast<int>(input.size());

    while (i < len) {
        const char c = next_char(input, i++);

        if (is_whitespace(c)) {
            continue;
        }

        if (c == '[' || c == ']' || c == '{' || c == '}'
            || c == ',' || c == ':') {
            res.push_back(sym_tok(c, i));
            continue;
        }
        if (c == '"') {
            i = detail::get_string_token(res, i, input);
            continue;
        }
        if (is_number_start(c)) {
            i = detail::get_num_token(res, i - 1, input);
            continue;
        }
        if (c == 't' || c == 'f' || c == 'n') {
            i = detail::get_keyword_token(res, i - 1, input);
            continue;
        }
        throw JsonError(input, i - 1, "Unexpected value");
    }
    return res;
}

namespace detail {

inline int get_string_token(
    std::vector<Token>& res, int start_pos, const std::string& input) {
    int close_pos = start_pos + 1;
    const int len = static_cast<int>(input.size());

    while (close_pos < len) {
        const char c = next_char(input, close_pos);
        if (c == '"' && next_char(input, close_pos - 1) != '\\') {
            break;
        }
        close_pos++;
    }
    if (close_pos == len) {
        throw JsonError(input, start_pos, "No closing quote character for string.");
    }

    std::string sb;
    int pos = start_pos;
    while (pos < close_pos) {
        char c = next_char(input, pos++);

        if (c < ' ') {
            throw JsonError(input, pos, "Invalid character in string");
        }

        if (c != '\\') {
            sb += c;
            continue;
        }
        assert(pos != close_pos);

        c = next_char(input, pos++);
        switch (c) {
        case '"':  sb += '"';  continue;
        case '\\': sb += '\\'; continue;
        case '/':  sb += '/';  continue;
        case 'b':  sb += '\b'; continue;
        case 'f':  sb += '\f'; continue;
        case 'n':  sb += '\n'; continue;
        case 'r':  sb += '\r'; continue;
        case 't':  sb += '\t'; continue;
        case 'u': {
            if (close_pos - pos < 4) {
                throw JsonError(input, pos,
                    "Unicode escape sequence is too short");
            }
            const std::string hex_str = input.substr(
                static_cast<std::size_t>(pos), 4);

            uint32_t unicode_val = 0;
            const auto [ptr, ec] = std::from_chars(
                hex_str.data(), hex_str.data() + hex_str.size(),
                unicode_val, 16);

            if (ec != std::errc{}) {
                throw JsonError(input, pos,
                    "Could not parse unicode escape sequence");
            }
            pos += 4;
            append_utf8(sb, unicode_val);
            continue;
        }
        default:
            continue;
        }
    }

    Token tok;
    tok.type = STRING_TYPE;
    tok.value = std::move(sb);
    tok.pos = start_pos;
    res.push_back(std::move(tok));
    return close_pos + 1;
}

inline std::pair<std::string, int> get_digits(
    int start_pos, const std::string& input) {
    std::string sb;
    int pos = start_pos;
    char c = next_char(input, pos++);
    while (c >= '0' && c <= '9') {
        sb += c;
        c = next_char(input, pos++);
    }
    return {sb, pos - 1};
}

inline int get_num_token(
    std::vector<Token>& res, int start_pos, const std::string& input) {
    int pos = start_pos;
    char c = next_char(input, pos);
    std::string num_str;

    if (c == '-') {
        num_str += c;
        pos++;
    }

    {
        const auto [digits, new_pos] = get_digits(pos, input);
        if (digits.empty()) {
            throw JsonError(input, pos, "Number format error");
        }
        num_str += digits;
        pos = new_pos;
    }

    c = next_char(input, pos);
    if (c == '.') {
        num_str += c;
        pos++;
        const auto [frac, new_pos] = get_digits(pos, input);
        if (frac.empty()) {
            throw JsonError(input, pos, "Number format error");
        }
        num_str += frac;
        pos = new_pos;
        c = next_char(input, pos);
    }

    if (c == 'e' || c == 'E') {
        num_str += c;
        pos++;
        c = next_char(input, pos);
        if (c == '+' || c == '-') {
            num_str += c;
            pos++;
        }
        const auto [exp_str, new_pos] = get_digits(pos, input);
        if (exp_str.empty()) {
            throw JsonError(input, pos, "Number format error");
        }
        num_str += exp_str;
        pos = new_pos;
    }

    Token tok;
    tok.type = NUM_TYPE;
    tok.value = std::move(num_str);
    tok.pos = start_pos;
    res.push_back(std::move(tok));
    return pos;
}

inline int get_keyword_token(
    std::vector<Token>& res, int pos, const std::string& input) {
    const char* keyword = nullptr;
    int end_pos = -1;
    const int len = static_cast<int>(input.size());

    if (len >= pos + 4 && input.substr(static_cast<std::size_t>(pos), 4) == "true") {
        keyword = "true";
        end_pos = pos + 4;
    }
    if (len >= pos + 5 && input.substr(static_cast<std::size_t>(pos), 5) == "false") {
        keyword = "false";
        end_pos = pos + 5;
    }
    if (len >= pos + 4 && input.substr(static_cast<std::size_t>(pos), 4) == "null") {
        keyword = "null";
        end_pos = pos + 4;
    }

    if (keyword == nullptr) {
        throw JsonError(input, pos, "Unexpected value");
    }

    Token tok;
    tok.type = KEYWORD_TYPE;
    tok.value = keyword;
    tok.pos = pos;
    res.push_back(std::move(tok));
    return end_pos;
}

}  // namespace detail

}  // namespace jaamsim::json

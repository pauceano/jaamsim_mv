#include "json/json_value.h"
#include "json/json_tokenizer.h"
#include "json/json_parser.h"
#include "json/json_writer.h"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    using namespace jaamsim::json;

    // Test 1: parse simple values
    {
        auto v = parse("42");
        assert(v.is_number());
        assert(v.num_val == 42.0);
    }
    {
        auto v = parse("-3.14");
        assert(v.is_number());
    }
    {
        auto v = parse("1.5e10");
        assert(v.is_number());
    }
    {
        auto v = parse("\"hello\"");
        assert(v.is_string());
        assert(v.str_val == "hello");
    }
    {
        auto v = parse("true");
        assert(v.is_bool() && v.is_true());
    }
    {
        auto v = parse("false");
        assert(v.is_bool() && v.is_false());
    }
    {
        auto v = parse("null");
        assert(v.is_null());
    }

    // Test 2: parse and write (roundtrip)
    {
        std::string input = R"({"name":"test","value":42,"flag":true})";
        auto v = parse(input);
        assert(v.is_object());
        assert(v.obj_val["name"].is_string());
        assert(v.obj_val["name"].str_val == "test");
        assert(v.obj_val["value"].is_number());
        assert(v.obj_val["value"].num_val == 42.0);
        assert(v.obj_val["flag"].is_bool());
        assert(v.obj_val["flag"].is_true());

        std::string output = to_json(v);
        auto v2 = parse(output);
        assert(v2.is_object());
        assert(v2.obj_val["name"].str_val == "test");
        assert(v2.obj_val["value"].num_val == 42.0);
        assert(v2.obj_val["flag"].is_true());
    }

    // Test 3: nested arrays
    {
        auto v = parse("[1,2,[3,4],5]");
        assert(v.is_array());
        assert(v.arr_val.size() == 4);
        assert(v.arr_val[2].is_array());
        assert(v.arr_val[2].arr_val.size() == 2);
    }

    // Test 4: escape sequences
    {
        auto v = parse(R"("\"hello\\world\n\u0041\u0042\u0043")");
        assert(v.is_string());
        assert(v.str_val == "\"hello\\world\nABC");
    }

    // Test 5: number formats
    {
        auto v = parse("0");
        assert(v.is_number());
        assert(v.num_val == 0.0);
    }
    {
        auto v = parse("1e2");
        assert(v.is_number());
        assert(v.num_val == 100.0);
    }

    // Test 6: writer produces valid JSON
    {
        Value obj = Value::make_object();
        obj.obj_val["a"] = Value::make_number(1.0);
        obj.obj_val["b"] = Value::make_string("hello");
        obj.obj_val["c"] = Value::make_bool(true);
        obj.obj_val["d"] = Value::make_null();
        obj.obj_val["e"] = Value::make_array();
        obj.obj_val["e"].arr_val.push_back(Value::make_number(99));

        std::string s = to_json(obj);
        auto v = parse(s);
        assert(v.is_object());
        assert(v.obj_val.size() == 5);
    }

    std::cout << "All tests passed.\n";
    return 0;
}

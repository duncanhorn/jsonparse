
#include <json_parser.h>

#include "test_guard.h"

using namespace std::literals;

static int parse_null_test() try
{
    test_guard guard{ "parse_null_test" };

    json::parse_null("null"); // Not much we can do with a nullptr_t...

    try
    {
        json::parse_null("null null");
        std::printf("ERROR: Expected an exception\n");
        return 1;
    }
    catch (...) {}

    return guard.success();
}
catch (std::exception& e)
{
    std::printf("ERROR: Unhandled exception %s\n", e.what());
    return 1;
}

static int parse_boolean_test() try
{
    test_guard guard{ "parse_boolean_test" };

    if (!json::parse_boolean("true"))
    {
        std::printf("ERROR: Incorrect return value; expected 'true'\n");
        return 1;
    }

    if (json::parse_boolean("false"))
    {
        std::printf("ERROR: Incorrect return value; expected 'false'\n");
        return 1;
    }

    try
    {
        json::parse_boolean("true false");
        std::printf("ERROR: Expected an exception\n");
        return 1;
    }
    catch (...) {}

    return guard.success();
}
catch (std::exception& e)
{
    std::printf("ERROR: Unhandled exception %s\n", e.what());
    return 1;
}

static int parse_string_test() try
{
    test_guard guard{ "parse_string_test" };

    auto str = json::parse_string("\"foo bar\"");
    if (str != "foo bar")
    {
        std::printf("ERROR: Incorrect string result; expected 'foo bar', got '%s'\n", str.c_str());
        return 1;
    }

    try
    {
        json::parse_string("\"foo\" \"bar\"");
        std::printf("ERROR: Expected an exception\n");
        return 1;
    }
    catch (...) {}

    return guard.success();
}
catch (std::exception& e)
{
    std::printf("ERROR: Unhandled exception %s\n", e.what());
    return 1;
}

static int parse_number_test() try
{
    test_guard guard{ "parse_number_test" };

    auto val = json::parse_number("0");
    if (val != 0)
    {
        std::printf("ERROR: Incorrect number result; expected 0, got %f\n", val);
        return 1;
    }

    val = json::parse_number("-0.0e+0");
    if (val != 0)
    {
        std::printf("ERROR: Incorrect number result; expected 0, got %f\n", val);
        return 1;
    }

    val = json::parse_number("-0.3e+1");
    if (val != -3)
    {
        std::printf("ERROR: Incorrect number result; expected -3, got %f\n", val);
        return 1;
    }

    val = json::parse_number("314159e-5");
    if (val != 3.14159)
    {
        std::printf("ERROR: Incorrect number result; expected 3.14159, got %f\n", val);
        return 1;
    }

    try
    {
        json::parse_number("0 0");
        std::printf("ERROR: Expected an exception\n");
        return 1;
    }
    catch (...) {}

    return guard.success();
}
catch (std::exception& e)
{
    std::printf("ERROR: Unhandled exception %s\n", e.what());
    return 1;
}

static int parse_array_test() try
{
    test_guard guard{ "parse_array_test" };

    auto check_size = [](json::array_t<>& arr, std::size_t expected)
    {
        if (arr.size() != expected)
        {
            std::printf("ERROR: Incorrect size; expected %zu, got %zu\n", expected, arr.size());
            return false;
        }
        return true;
    };

    auto check_type = [](json::value& v, json::value_type type)
    {
        if (v.type() != type)
        {
            std::printf("Incorrect value type\n");
            return false;
        }
        return true;
    };

    auto arr = json::parse_array("[]");
    if (!check_size(arr, 0)) return 1;

    arr = json::parse_array("[ 42,true,false,null,\"foo\" ]");
    if (!check_size(arr, 5)) return 1;
    if (!check_type(arr[0], json::value_type::number)) return 1;
    if (arr[0].number() != 42)
    {
        std::printf("ERROR: Incorrect number; expected 42, got %f\n", arr[0].number());
        return 1;
    }
    if (!check_type(arr[1], json::value_type::boolean)) return 1;
    if (!arr[1].boolean())
    {
        std::printf("ERROR: Incorrect boolean; expected true\n");
        return 1;
    }
    if (!check_type(arr[2], json::value_type::boolean)) return 1;
    if (arr[2].boolean())
    {
        std::printf("ERROR: Incorrect boolean; expected false\n");
        return 1;
    }
    if (!check_type(arr[3], json::value_type::null)) return 1;
    if (!check_type(arr[4], json::value_type::string)) return 1;
    if (arr[4].string() != "foo")
    {
        std::printf("ERROR: Incorrect string; expected 'foo', got '%s'\n", arr[4].string().c_str());
        return 1;
    }

    try
    {
        json::parse_array("[] []");
        std::printf("ERROR: Expected an exception\n");
        return 1;
    }
    catch (...) {}

    return guard.success();
}
catch (std::exception& e)
{
    std::printf("ERROR: Unhandled exception %s\n", e.what());
    return 1;
}

static int parse_object_test() try
{
    test_guard guard{ "parse_object_test" };

    auto check_size = [](json::object_t& obj, std::size_t expected)
    {
        if (obj.size() != expected)
        {
            std::printf("ERROR: Incorrect size; expected %zu, got %zu\n", expected, obj.size());
            return false;
        }
        return true;
    };

    auto check_type = [](json::object_t& obj, std::string_view key, json::value_type expected)
    {
        auto itr = obj.find(key);
        if (itr == obj.end())
        {
            std::printf("ERROR: Key '%s' not found in object\n", key.data());
            return false;
        }
        else if (itr->second.type() != expected)
        {
            std::printf("ERROR: Incorrect type\n");
            return false;
        }
        return true;
    };

    auto obj = json::parse_object("{}");
    if (!check_size(obj, 0)) return 1;

    obj = json::parse_object("{ \"answer\": 42, \"foo\": \"bar\", \"success\": true }");
    if (!check_size(obj, 3)) return 1;
    if (!check_type(obj, "answer", json::value_type::number)) return 1;
    if (json::object_get(obj, "answer").number() != 42)
    {
        std::printf("ERROR: Incorrect number; expected 42, but got %f\n", json::object_get(obj, "answer").number());
        return 1;
    }
    if (!check_type(obj, "foo", json::value_type::string)) return 1;
    if (!check_type(obj, "success", json::value_type::boolean)) return 1;

    obj = json::parse_object("{ \"x\": { \"y\": { \"z\": 42 } } }");
    if (!check_size(obj, 1)) return 1;
    if (!check_type(obj, "x", json::value_type::object)) return 1;

    try
    {
        json::parse_array("{} {}");
        std::printf("ERROR: Expected an exception\n");
        return 1;
    }
    catch (...) {}

    return guard.success();
}
catch (std::exception& e)
{
    std::printf("ERROR: Unhandled exception %s\n", e.what());
    return 1;
}

int parser_tests()
{
    int result = 0;
    result += parse_null_test();
    result += parse_boolean_test();
    result += parse_string_test();
    result += parse_number_test();
    result += parse_array_test();
    result += parse_object_test();
    return result;
}

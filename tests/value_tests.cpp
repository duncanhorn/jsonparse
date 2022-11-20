
#include <json.h>
#include <sstream>

#include "test_guard.h"

using namespace std::literals;

template <typename Callback>
static bool run_with_lexer(const std::string& str, Callback&& callback)
{
    json::buffer_input_stream bufferStream(str.data(), str.data() + str.size());
    json::lexer bufferLexer(bufferStream);
    if (!callback(bufferLexer)) return false;

    std::stringstream sstream(str);
    json::istream istream(sstream);
    json::lexer istreamLexer(istream);
    if (!callback(istreamLexer)) return false;

    return true;
}

bool check_value(const json::value& value, const json::value& expected);

template <typename T>
bool check_value(const json::value& value, const T& expected)
{
    if (!std::holds_alternative<T>(value.data))
    {
        std::printf("ERROR: Value does not hold type %s\n", typeid(T).name());
        return false;
    }

    auto& tgt = std::get<T>(value.data);
    if constexpr (std::is_same_v<T, json::object>)
    {
        if (tgt.size() != expected.size())
        {
            std::printf("ERROR: Object does not hold the expected number of members\n");
            return false;
        }

        for (auto&& pair : expected)
        {
            auto member = json::object_get(tgt, pair.first);
            if (!member)
            {
                std::printf("ERROR: Object does not hold member '%s'\n", pair.first.c_str());
                return false;
            }

            if (!check_value(*member, pair.second)) return false;
        }
    }
    else if constexpr (std::is_same_v<T, json::array>)
    {
        if (tgt.size() != expected.size())
        {
            std::printf("ERROR: Array does not hold the expected number of values\n");
            return false;
        }

        for (std::size_t i = 0; i < tgt.size(); ++i)
        {
            if (!check_value(tgt[i], expected[i])) return false;
        }
    }
    else if (tgt != expected)
    {
        std::printf("ERROR: Invalid value\n");
        return false;
    }

    return true;
}

bool check_value(const json::value& value, const json::value& expected)
{
    if (auto null = expected.get_null())
    {
        return check_value(value, *null);
    }
    else if (auto boolean = expected.get_boolean())
    {
        return check_value(value, *boolean);
    }
    else if (auto number = expected.get_number())
    {
        return check_value(value, *number);
    }
    else if (auto string = expected.get_string())
    {
        return check_value(value, *string);
    }
    else if (auto array = expected.get_array())
    {
        return check_value(value, *array);
    }
    else if (auto object = expected.get_object())
    {
        return check_value(value, *object);
    }

    return false;
}

template <typename T>
bool check_value(const json::object& obj, std::string_view name, const T& expected)
{
    auto value = json::object_get(obj, name);
    if (!value)
    {
        std::printf("ERROR: '%.*s' not found in object\n", static_cast<int>(name.size()), name.data());
        return false;
    }

    return check_value(*value, expected);
}

static int parse_value_test()
{
    test_guard guard{ "parse_value_test" };

    if (!run_with_lexer(R"^-^({
        "null": null,
        "true": true,
        "false": false,
        "number": 42,
        "string": "string",
        "array": [ true, false, [ null ] ],
        "object": {
            "sub-null": null,
            "sub-string": "string",
            "sub-array": [ true, "foo" ]
        }
    })^-^",
            [](auto& lexer) {
                json::value value;
                if (!json::parse_value(lexer, value))
                {
                    std::printf("ERROR: parse_value failed\n");
                    return false;
                }

                if (!check_value(value,
                        json::object{ { "null", nullptr }, { "true", true }, { "false", false }, { "number", 42. },
                            { "string", "string" }, { "array", json::array{ true, false, json::array{ nullptr } } },
                            { "object",
                                json::object{ { "sub-null", nullptr }, { "sub-string", "string" },
                                    { "sub-array", json::array{ true, "foo" } } } } }))
                    return false;

                return true;
            }))
        return 1;

    return guard.success();
}

int value_tests()
{
    int result = 0;
    result += parse_value_test();
    return result;
}


#include <json_parser.h>
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

static int parse_null_test()
{
    test_guard guard{ "parse_null_test" };

    auto do_test = [](const std::string& str, bool expectSuccess) {
        return run_with_lexer(str,
                   [&](auto& lexer) {
                       int dummy = 42;
                       int* ptr = &dummy;
                       auto result = json::parse_null(lexer, ptr);
                       if (result != expectSuccess)
                       {
                           std::printf("ERROR: Incorrectly parsed '%s'\n", str.c_str());
                           return false;
                       }

                       if (expectSuccess && ptr)
                       {
                           std::printf("ERROR: Pointer not set to null\n");
                           return false;
                       }
                       else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                       {
                           std::printf("ERROR: Not all tokens read\n");
                           return false;
                       }

                       return true;
                   }) &&
            run_with_lexer(str, [&](auto& lexer) {
                if (json::ignore_null(lexer) != expectSuccess)
                {
                    std::printf("ERROR: ignore_null failed\n");
                    return false;
                }
                else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                {
                    std::printf("ERROR: Not all tokens read\n");
                    return false;
                }

                return true;
            });
    };

    if (!do_test("null", true)) return 1;
    if (!do_test("true", false)) return 1;
    if (!do_test("[null]", false)) return 1;
    if (!do_test("invalid", false)) return 1;

    return guard.success();
}

static int parse_true_test()
{
    test_guard guard{ "parse_true_test" };

    auto do_test = [&](const std::string& str, bool expectSuccess) {
        return run_with_lexer(str,
                   [&](auto& lexer) {
                       bool value = false;
                       auto result = json::parse_true(lexer, value);
                       if (result != expectSuccess)
                       {
                           std::printf("ERROR: Incorrectly parsed '%s'\n", str.c_str());
                           return false;
                       }

                       if (expectSuccess && !value)
                       {
                           std::printf("ERROR: Boolean not set to true\n");
                           return false;
                       }
                       else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                       {
                           std::printf("ERROR: Not all tokens read\n");
                           return false;
                       }

                       return true;
                   }) &&
            run_with_lexer(str, [&](auto& lexer) {
                if (json::ignore_true(lexer) != expectSuccess)
                {
                    std::printf("ERROR: ignore_true failed\n");
                    return false;
                }
                else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                {
                    std::printf("ERROR: Not all tokens read\n");
                    return false;
                }

                return true;
            });
    };

    if (!do_test("true", true)) return false;
    if (!do_test("false", false)) return false;
    if (!do_test("[true]", false)) return false;
    if (!do_test("invalid", false)) return false;

    return guard.success();
}

static int parse_false_test()
{
    test_guard guard{ "parse_false_test" };

    auto do_test = [&](const std::string& str, bool expectSuccess) {
        return run_with_lexer(str,
                   [&](auto& lexer) {
                       bool value = true;
                       auto result = json::parse_false(lexer, value);
                       if (result != expectSuccess)
                       {
                           std::printf("ERROR: Incorrectly parsed '%s'\n", str.c_str());
                           return false;
                       }

                       if (expectSuccess && value)
                       {
                           std::printf("ERROR: Boolean not set to false\n");
                           return false;
                       }
                       else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                       {
                           std::printf("ERROR: Not all tokens read\n");
                           return false;
                       }

                       return true;
                   }) &&
            run_with_lexer(str, [&](auto& lexer) {
                if (json::ignore_false(lexer) != expectSuccess)
                {
                    std::printf("ERROR: ignore_false failed\n");
                    return false;
                }
                else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                {
                    std::printf("ERROR: Not all tokens read\n");
                    return false;
                }

                return true;
            });
    };

    if (!do_test("false", true)) return false;
    if (!do_test("true", false)) return false;
    if (!do_test("[false]", false)) return false;
    if (!do_test("invalid", false)) return false;

    return guard.success();
}

static int parse_bool_test()
{
    test_guard guard{ "parse_bool_test" };

    auto do_test = [&](const std::string& str, bool expectSuccess, bool expectedValue = false) {
        return run_with_lexer(str,
                   [&](auto& lexer) {
                       bool value = true;
                       auto result = json::parse_bool(lexer, value);
                       if (result != expectSuccess)
                       {
                           std::printf("ERROR: Incorrectly parsed '%s'\n", str.c_str());
                           return false;
                       }

                       if (expectSuccess && (value != expectedValue))
                       {
                           std::printf("ERROR: Boolean not set correctly\n");
                           return false;
                       }
                       else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                       {
                           std::printf("ERROR: Not all tokens read\n");
                           return false;
                       }

                       return true;
                   }) &&
            run_with_lexer(str, [&](auto& lexer) {
                if (json::ignore_bool(lexer) != expectSuccess)
                {
                    std::printf("ERROR: ignore_bool failed\n");
                    return false;
                }
                else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                {
                    std::printf("ERROR: Not all tokens read\n");
                    return false;
                }

                return true;
            });
    };

    if (!do_test("true", true, true)) return false;
    if (!do_test("false", true, false)) return false;
    if (!do_test("null", false)) return false;
    if (!do_test("[true]", false)) return false;
    if (!do_test("[false]", false)) return false;
    if (!do_test("invalid", false)) return false;

    return guard.success();
}

static int parse_string_test()
{
    test_guard guard{ "parse_string_test" };

    auto do_test = [&](const std::string& str, bool expectSuccess, const std::string& expectedValue = "") {
        return run_with_lexer(str,
                   [&](auto& lexer) {
                       std::string value;
                       auto result = json::parse_string(lexer, value);
                       if (result != expectSuccess)
                       {
                           std::printf("ERROR: Incorrectly parsed '%s'\n", str.c_str());
                           return false;
                       }

                       if (expectSuccess && (value != expectedValue))
                       {
                           std::printf("ERROR: String not set correctly\n");
                           return false;
                       }
                       else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                       {
                           std::printf("ERROR: Not all tokens read\n");
                           return false;
                       }

                       return true;
                   }) &&
            run_with_lexer(str, [&](auto& lexer) {
                if (json::ignore_string(lexer) != expectSuccess)
                {
                    std::printf("ERROR: ignore_string failed\n");
                    return false;
                }
                else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                {
                    std::printf("ERROR: Not all tokens read\n");
                    return false;
                }

                return true;
            });
    };

    if (!do_test("\"\"", true, "")) return false;
    if (!do_test("\"foo\"", true, "foo")) return false;
    if (!do_test("foo", false)) return false;
    if (!do_test("null", false)) return false;
    if (!do_test("[\"foo\"]", false)) return false;
    if (!do_test("{\"foo\":\"bar\"}", false)) return false;

    return guard.success();
}

static int parse_number_test()
{
    test_guard guard{ "parse_number_test" };

    auto do_test = [](const std::string& str, bool expectSuccess, auto expectedValue = 0) {
        return run_with_lexer(str,
                   [&](auto& lexer) {
                       decltype(expectedValue) value;
                       auto result = json::parse_number(lexer, value);
                       if (result != expectSuccess)
                       {
                           std::printf("ERROR: Incorrectly parsed '%s'\n", str.c_str());
                           return false;
                       }

                       if (expectSuccess && (value != expectedValue))
                       {
                           std::printf("ERROR: Number not set correctly\n");
                           return false;
                       }
                       else if (expectSuccess && (lexer.current_token != json::lexer_token::eof))
                       {
                           std::printf("ERROR: Not all tokens read\n");
                           return false;
                       }

                       return true;
                   }) &&
            run_with_lexer(str, [](auto& lexer) {
                // NOTE: All arguments will be valid "numbers" we might just be expecting to fail parsing them
                if (!json::ignore_number(lexer))
                {
                    std::printf("ERROR: ignore_number failed\n");
                    return false;
                }
                else if (lexer.current_token != json::lexer_token::eof)
                {
                    std::printf("ERROR: Not all tokens read\n");
                    return false;
                }

                return true;
            });
    };

    if (!do_test("0", true, 0)) return 1;
    if (!do_test("0", true, 0.)) return 1;
    if (!do_test("-0", true, 0)) return 1;
    if (!do_test("-0", true, -0.)) return 1;
    if (!do_test("0.000", true, 0)) return 1;
    if (!do_test("0.000e100", true, 0)) return 1;
    if (!do_test("0.000e-100", true, 0)) return 1;

    if (!do_test("42", true, 42)) return 1;
    if (!do_test("-42", true, -42)) return 1;
    if (!do_test("42", true, 42u)) return 1;
    if (!do_test("42", true, 42ll)) return 1;
    if (!do_test("-42", true, -42ll)) return 1;
    if (!do_test("42", true, 42ull)) return 1;
    if (!do_test("42", true, 42.)) return 1;
    if (!do_test("-42", true, -42.)) return 1;
    if (!do_test("42", true, 42.f)) return 1;
    if (!do_test("-42", true, -42.f)) return 1;
    if (!do_test("42", true, static_cast<char>(42))) return 1;
    if (!do_test("-42", true, static_cast<std::int8_t>(-42))) return 1;

    if (!do_test("42e0", true, 42)) return 1;
    if (!do_test("42e0", true, 42ll)) return 1;
    if (!do_test("42e0", true, 42ull)) return 1;
    if (!do_test("42.0e0", true, 42)) return 1;
    if (!do_test("42.0e0", true, 42ll)) return 1;
    if (!do_test("42.0e0", true, 42ull)) return 1;
    if (!do_test("4.2e1", true, 42)) return 1;
    if (!do_test("4.2e1", true, 42ll)) return 1;
    if (!do_test("4.2e1", true, 42ull)) return 1;
    if (!do_test("420e-1", true, 42)) return 1;
    if (!do_test("420e-1", true, 42ll)) return 1;
    if (!do_test("420e-1", true, 42ull)) return 1;
    if (!do_test("4.2000000000000000000000000000000e1", true, 42)) return 1;
    if (!do_test("4.2000000000000000000000000000000e1", true, 42ll)) return 1;
    if (!do_test("4.2000000000000000000000000000000e1", true, 42ull)) return 1;
    if (!do_test("0.0000000000000000000000000000042e31", true, 42)) return 1;
    if (!do_test("0.0000000000000000000000000000042e31", true, 42ll)) return 1;
    if (!do_test("0.0000000000000000000000000000042e31", true, 42ull)) return 1;

    // Largest signed/unsigned 8-bit integer is 127/255
    if (!do_test("127", true, static_cast<std::int8_t>(127))) return 1;
    if (!do_test("1.27e2", true, static_cast<std::int8_t>(127))) return 1;
    if (!do_test("0.00127e5", true, static_cast<std::int8_t>(127))) return 1;
    if (!do_test("127000e-3", true, static_cast<std::int8_t>(127))) return 1;
    if (!do_test("128", false, static_cast<std::int8_t>(0))) return 1;
    if (!do_test("1.28e2", false, static_cast<std::int8_t>(0))) return 1;
    if (!do_test("0.00128e5", false, static_cast<std::int8_t>(0))) return 1;
    if (!do_test("128000e-3", false, static_cast<std::int8_t>(0))) return 1;

    if (!do_test("-128", true, static_cast<std::int8_t>(-128))) return 1;
    if (!do_test("-1.28e2", true, static_cast<std::int8_t>(-128))) return 1;
    if (!do_test("-0.00128e5", true, static_cast<std::int8_t>(-128))) return 1;
    if (!do_test("-128000e-3", true, static_cast<std::int8_t>(-128))) return 1;
    if (!do_test("-129", false, static_cast<std::int8_t>(0))) return 1;
    if (!do_test("-1.29e2", false, static_cast<std::int8_t>(0))) return 1;
    if (!do_test("-0.00129e5", false, static_cast<std::int8_t>(0))) return 1;
    if (!do_test("-129000e-3", false, static_cast<std::int8_t>(0))) return 1;

    if (!do_test("255", true, static_cast<std::uint8_t>(255))) return 1;
    if (!do_test("2.55e2", true, static_cast<std::uint8_t>(255))) return 1;
    if (!do_test("0.00255e5", true, static_cast<std::uint8_t>(255))) return 1;
    if (!do_test("255000e-3", true, static_cast<std::uint8_t>(255))) return 1;
    if (!do_test("256", false, static_cast<std::uint8_t>(0))) return 1;
    if (!do_test("2.56e2", false, static_cast<std::uint8_t>(0))) return 1;
    if (!do_test("0.00256e5", false, static_cast<std::uint8_t>(0))) return 1;
    if (!do_test("256000e-3", false, static_cast<std::uint8_t>(0))) return 1;

    // Largest signed/unsigned 32-bit integer is 2147483647/4294967295
    if (!do_test("2147483647", true, 2147483647)) return 1;
    if (!do_test("2.147483647e9", true, 2147483647)) return 1;
    if (!do_test("0.002147483647e12", true, 2147483647)) return 1;
    if (!do_test("2147483647000e-3", true, 2147483647)) return 1;
    if (!do_test("2147483648", false, 0)) return 1;
    if (!do_test("2.147483648e9", false, 0)) return 1;
    if (!do_test("0.002147483648e12", false, 0)) return 1;
    if (!do_test("2147483648000e-3", false, 0)) return 1;

    if (!do_test("-2147483648", true, -2147483648)) return 1;
    if (!do_test("-2.147483648e9", true, -2147483648)) return 1;
    if (!do_test("-0.002147483648e12", true, -2147483648)) return 1;
    if (!do_test("-2147483648000e-3", true, -2147483648)) return 1;
    if (!do_test("-2147483649", false, 0)) return 1;
    if (!do_test("-2.147483649e9", false, 0)) return 1;
    if (!do_test("-0.002147483649e12", false, 0)) return 1;
    if (!do_test("-2147483649000e-3", false, 0)) return 1;

    if (!do_test("4294967295", true, 4294967295u)) return 1;
    if (!do_test("4.294967295e9", true, 4294967295u)) return 1;
    if (!do_test("0.004294967295e12", true, 4294967295u)) return 1;
    if (!do_test("4294967295000e-3", true, 4294967295u)) return 1;
    if (!do_test("4294967296", false, 0u)) return 1;
    if (!do_test("4.294967296e9", false, 0u)) return 1;
    if (!do_test("0.004294967296e12", false, 0u)) return 1;
    if (!do_test("4294967296000e-3", false, 0u)) return 1;

    // Largest signed/unsigned 64-bit integer is 9223372036854775807/18446744073709551615
    if (!do_test("9223372036854775807", true, 9223372036854775807ll)) return 1;
    if (!do_test("9.223372036854775807e18", true, 9223372036854775807ll)) return 1;
    if (!do_test("0.009223372036854775807e21", true, 9223372036854775807ll)) return 1;
    if (!do_test("9223372036854775807000e-3", true, 9223372036854775807ll)) return 1;
    if (!do_test("9223372036854775808", false, 0ll)) return 1;
    if (!do_test("9.223372036854775808e18", false, 0ll)) return 1;
    if (!do_test("0.009223372036854775808e21", false, 0ll)) return 1;
    if (!do_test("9223372036854775808000e-3", false, 0ll)) return 1;

    if (!do_test("-9223372036854775808", true, -9223372036854775807ll - 1)) return 1;
    if (!do_test("-9.223372036854775808e18", true, -9223372036854775807ll - 1)) return 1;
    if (!do_test("-0.009223372036854775808e21", true, -9223372036854775807ll - 1)) return 1;
    if (!do_test("-9223372036854775808000e-3", true, -9223372036854775807ll - 1)) return 1;
    if (!do_test("-9223372036854775809", false, 0ll)) return 1;
    if (!do_test("-9.223372036854775809e18", false, 0ll)) return 1;
    if (!do_test("-0.009223372036854775809e21", false, 0ll)) return 1;
    if (!do_test("-9223372036854775809000e-3", false, 0ll)) return 1;

    if (!do_test("18446744073709551615", true, 18446744073709551615ull)) return 1;
    if (!do_test("1.8446744073709551615e19", true, 18446744073709551615ull)) return 1;
    if (!do_test("0.0018446744073709551615e22", true, 18446744073709551615ull)) return 1;
    if (!do_test("18446744073709551615000e-3", true, 18446744073709551615ull)) return 1;
    if (!do_test("18446744073709551616", false, 0ull)) return 1;
    if (!do_test("1.8446744073709551616e19", false, 0ull)) return 1;
    if (!do_test("0.0018446744073709551616e22", false, 0ull)) return 1;
    if (!do_test("18446744073709551616000e-3", false, 0ull)) return 1;

    // Fractional components for integers should cause failures
    if (!do_test("123e-1", false, static_cast<std::int8_t>(0))) return 1;
    if (!do_test("123e-1", false, static_cast<std::uint8_t>(0))) return 1;
    if (!do_test("123e-1", false, static_cast<std::int16_t>(0))) return 1;
    if (!do_test("123e-1", false, static_cast<std::uint16_t>(0))) return 1;
    if (!do_test("123e-1", false, static_cast<std::int32_t>(0))) return 1;
    if (!do_test("123e-1", false, static_cast<std::uint32_t>(0))) return 1;
    if (!do_test("123e-1", false, static_cast<std::int64_t>(0))) return 1;
    if (!do_test("123e-1", false, static_cast<std::uint64_t>(0))) return 1;

    return guard.success();
}

static int parse_array_test()
{
    test_guard guard{ "parse_array_test" };

    if (!run_with_lexer("[]", [](auto& lexer) {
            int* dummyArray;
            return json::parse_array(lexer, dummyArray, [](auto&&, auto&&) {
                std::printf("ERROR: Array should be empty\n");
                return false; // Should never see a callback
            });
        }))
        return 1;

    if (!run_with_lexer("[ true ]", [](auto& lexer) {
            bool lastValue = false;
            return json::parse_array(lexer, &lastValue, [](auto& lexer, bool* lastValue) {
                if (*lastValue || (lexer.current_token != json::lexer_token::keyword_true))
                {
                    std::printf("ERROR: Array should have a single true\n");
                    return false;
                }

                *lastValue = true;
                lexer.advance();
                return true;
            });
        }))
        return 1;

    if (!run_with_lexer("[true,false, true,   false]", [](auto& lexer) {
            int count = 0;
            if (!json::parse_array(lexer, &count, [](auto& lexer, int* count) {
                    bool value;
                    if (!json::parse_bool(lexer, value))
                    {
                        std::printf("ERROR: Array should only contain boolean values\n");
                        return false;
                    }

                    if ((*count)++ >= 4)
                    {
                        std::printf("ERROR: Array should only have 4 values\n");
                        return false;
                    }

                    if (value != ((*count % 2) == 1))
                    {
                        std::printf("ERROR: Incorrect boolean value\n");
                        return false;
                    }

                    return true;
                }))
                return false;

            return count == 4;
        }))
        return 1;

    if (!run_with_lexer("[  ", [](auto& lexer) {
            bool sawCallback = false;
            if (json::parse_array(lexer, nullptr, [&](auto&&, auto&&) {
                    sawCallback = true;
                    std::printf("ERROR: Expected no callbacks\n");
                    return true; // Continue processing; we're already going to fail so see if we fail in other places
                }))
            {
                std::printf("ERROR: Expected failure\n");
                return false;
            }
            else if (sawCallback)
                return false; // Already displayed error

            return true;
        }))
        return 1;

    if (!run_with_lexer("[null  ", [](auto& lexer) {
            int callbackCount = 0;
            if (json::parse_array(lexer, nullptr, [&](auto&&, auto&&) {
                    ++callbackCount;
                    int* dummy;
                    return json::parse_null(lexer, dummy);
                }))
            {
                std::printf("ERROR: Expected failure\n");
                return false;
            }
            else if (callbackCount != 1)
            {
                std::printf("ERROR: Callback invoked an unexpected number of times\n");
                return false;
            }

            return true;
        }))
        return 1;

    if (!run_with_lexer("\"[]\"", [](auto& lexer) {
            bool sawCallback = false;
            if (json::parse_array(lexer, nullptr, [&](auto&&, auto&&) {
                    sawCallback = true;
                    std::printf("ERROR: Expected no callbacks\n");
                    return true; // Continue processing to see if we hit more errors
                }))
            {
                std::printf("ERROR: Expected failure\n");
                return false;
            }
            else if (sawCallback)
                return false;

            return true;
        }))
        return 1;

    if (!run_with_lexer("[]", [](auto& lexer) { return json::ignore_array(lexer); }))
    {
        std::printf("ERRROR: ignore_array failed\n");
        return 1;
    }

    if (!run_with_lexer("[ true, false, null, \"foo\", 1.234, { \"foo\": \"bar\" }, [ 1, 2, 3 ] ]",
            [](auto& lexer) { return json::ignore_array(lexer); }))
    {
        std::printf("ERRROR: ignore_array failed\n");
        return 1;
    }

    return guard.success();
}

static int parse_object_test()
{
    test_guard guard{ "parse_object_test" };

    if (!run_with_lexer("{}", [](auto& lexer) {
            return json::parse_object(lexer, nullptr, [](auto&&, auto&&, auto&&) {
                std::printf("ERROR: Expected no object members\n");
                return false;
            });
        }))
        return 1;

    if (!run_with_lexer("{\"value\": true}", [](auto& lexer) {
            bool value = false;
            if (!json::parse_object(lexer, nullptr, [&](auto& lexer, auto&&, auto&& name) {
                    if (name != "value")
                    {
                        std::printf("ERROR: Invalid member name '%s'\n", name.c_str());
                        return false;
                    }

                    return json::parse_bool(lexer, value);
                }))
                return false;

            if (!value)
            {
                std::printf("ERROR: Value not set correctly\n");
                return false;
            }

            return true;
        }))
        return 1;

    if (!run_with_lexer("{\"a\":\"a\", \"b\":\"b\"}", [](auto& lexer) {
            bool saw[] = { false, false };
            if (!json::parse_object(lexer, nullptr, [&](auto& lexer, auto&&, auto&& name) {
                    int index = 0;
                    const char* expected = "";
                    if (name == "a")
                    {
                        index = 0;
                        expected = "a";
                    }
                    else if (name == "b")
                    {
                        index = 1;
                        expected = "b";
                    }
                    else
                    {
                        std::printf("ERROR: Invalid member name '%s'\n", name.c_str());
                        return false;
                    }

                    std::string value;
                    if (saw[index])
                    {
                        std::printf("ERROR: Encountered multiple values for '%s'\n", expected);
                        return false;
                    }
                    else if (!json::parse_string(lexer, value))
                        return false;
                    else if (value != expected)
                    {
                        std::printf("ERROR: Value '%s' did not match the expected value of '%s'\n", value.c_str(),
                            expected);
                        return false;
                    }

                    saw[index] = true;
                    return true;
                }))
                return false;

            if (!saw[0] || !saw[1])
            {
                std::printf("ERROR: Not all members encountered\n");
                return false;
            }

            return true;
        }))
        return 1;

    if (!run_with_lexer("{  ", [](auto& lexer) {
            bool sawCallback = false;
            if (json::parse_object(lexer, nullptr, [&](auto&&, auto&&, auto&&) {
                    std::printf("ERROR: Expected no callbacks\n");
                    sawCallback = true;
                    return true;
                }))
            {
                std::printf("ERROR: Expected failure\n");
                return false;
            }
            else if (sawCallback)
                return false; // Already printed error

            return true;
        }))
        return 1;

    if (!run_with_lexer("{\"foo\":null  ", [](auto& lexer) {
            int callbackCount = 0;
            if (json::parse_object(lexer, nullptr, [&](auto& lexer, auto&&, auto&&) {
                    ++callbackCount;
                    int* dummy;
                    return json::parse_null(lexer, dummy);
                }))
            {
                std::printf("ERROR: Expected failure\n");
                return false;
            }
            else if (callbackCount != 1)
            {
                std::printf("ERROR: Callback called more than once\n");
                return false;
            }

            return true;
        }))
        return 1;

    if (!run_with_lexer("{null:null}", [](auto& lexer) {
            bool sawCallback = false;
            if (json::parse_object(lexer, nullptr, [&](auto&&, auto&&, auto&&) {
                    sawCallback = true;
                    std::printf("ERROR: Expected no callbacks\n");
                    return true;
                }))
            {
                std::printf("ERROR: Expected failure\n");
                return false;
            }
            else if (sawCallback)
                return false;

            return true;
        }))
        return 1;

    if (!run_with_lexer("{\"foo\":  ", [](auto& lexer) {
            bool sawCallback = false;
            if (json::parse_object(lexer, nullptr, [&](auto&&, auto&&, auto&&) {
                    sawCallback = true;
                    std::printf("ERROR: Expected no callbacks\n");
                    return true;
                }))
            {
                std::printf("ERROR: Expected failure\n");
                return false;
            }
            else if (sawCallback)
                return false;

            return true;
        }))
        return 1;

    if (!run_with_lexer("{\"foo\": invalid  ", [](auto& lexer) {
            bool sawCallback = false;
            if (json::parse_object(lexer, nullptr, [&](auto&&, auto&&, auto&&) {
                    sawCallback = true;
                    std::printf("ERROR: Expected no callbacks\n");
                    return true;
                }))
            {
                std::printf("ERROR: Expected failure\n");
                return false;
            }
            else if (sawCallback)
                return false;

            return true;
        }))
        return 1;

    if (!run_with_lexer("{}", [](auto& lexer) { return json::ignore_object(lexer); }))
    {
        std::printf("ERROR: ignore_object failed\n");
        return 1;
    }

    if (!run_with_lexer(R"^-^({
        "null": null,
        "true": true,
        "false": false,
        "string": "string",
        "number": 8,
        "array": [ null, true, false, "string", [ "array" ], { "object": true } ],
        "object": {
            "null": null,
            "array": [ null, [ "sub-array" ], { "object": { "hello": "world" }, "foo": "bar" } ],
            "sub-object": { "bool": true, "number": 42 }
        }
    })^-^",
            [](auto& lexer) { return json::ignore_object(lexer); }))
    {
        std::printf("ERROR: ignore_object failed\n");
        return 1;
    }

    return guard.success();
}

int parser_tests()
{
    int result = 0;
    result += parse_null_test();
    result += parse_true_test();
    result += parse_false_test();
    result += parse_bool_test();
    result += parse_string_test();
    result += parse_number_test();
    result += parse_array_test();
    result += parse_object_test();
    return result;
}

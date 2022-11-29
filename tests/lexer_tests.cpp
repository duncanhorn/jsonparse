
#include <json_lexer.h>
#include <sstream>

#include "test_guard.h"

using namespace std::literals;

template <json::Utf8Char CharT, typename InputStream>
static bool test_lex_expect_tokens(json::lexer<InputStream>& lexer,
    std::initializer_list<std::pair<json::lexer_token, std::basic_string_view<CharT>>> expected)
{
    bool expectInvalid = false;
    for (auto&& pair : expected)
    {
        if (lexer.current_token != pair.first)
        {
            std::printf("ERROR: Incorrect token. Expected '%d', got '%d'\n", pair.first, lexer.current_token);
            return false;
        }
        else if (lexer.string_value != pair.second)
        {
            std::printf("ERROR: Incorrect token string. Expected '%.*s', got '%s'\n",
                static_cast<int>(pair.second.size()), reinterpret_cast<const char*>(pair.second.data()),
                reinterpret_cast<const char*>(lexer.string_value.c_str()));
            return false;
        }

        expectInvalid = (pair.first == json::lexer_token::invalid);
        lexer.advance();
    }

    if (!expectInvalid && (lexer.current_token != json::lexer_token::eof))
    {
        std::printf("ERROR: Incorrect token. Expected 'eof', got '%d'\n", lexer.current_token);
        return false;
    }

    return true;
}

template <json::Utf8Char CharT, typename InputStream>
static bool test_lex_expect_single_token(json::lexer<InputStream>& lexer, json::lexer_token expected,
    std::basic_string_view<CharT> expectedStr)
{
    return test_lex_expect_tokens(lexer, { std::pair{ expected, expectedStr } });
}

template <json::Utf8Char CharT>
static bool test_lex_expect_tokens(const CharT* begin, const CharT* end,
    std::initializer_list<std::pair<json::lexer_token, std::basic_string_view<CharT>>> expected)
{
    json::buffer_input_stream stream(begin, end);
    json::lexer lexer(stream);
    return test_lex_expect_tokens(lexer, expected);
}

template <json::Utf8Char CharT>
static bool test_lex_expect_single_token(const CharT* begin, const CharT* end, json::lexer_token expected,
    std::basic_string_view<CharT> expectedStr)
{
    json::buffer_input_stream stream(begin, end);
    json::lexer lexer(stream);
    return test_lex_expect_single_token(lexer, expected, expectedStr);
}

template <json::Utf8Char CharT>
static bool test_lex_expect_tokens(std::basic_stringstream<CharT> input,
    std::initializer_list<std::pair<json::lexer_token, std::basic_string_view<CharT>>> expected)
{
    json::istream stream(input);
    json::lexer lexer(stream);
    return test_lex_expect_tokens(lexer, expected);
}

template <json::Utf8Char CharT>
static bool test_lex_expect_single_token(std::basic_stringstream<CharT> input, json::lexer_token expected,
    std::basic_string_view<CharT> expectedStr)
{
    json::istream stream(input);
    json::lexer lexer(stream);
    return test_lex_expect_single_token(lexer, expected, expectedStr);
}

template <json::Utf8Char CharT>
static bool test_lex_expect_tokens(const std::basic_string<CharT>& str,
    std::initializer_list<std::pair<json::lexer_token, std::basic_string_view<CharT>>> expected)
{
    return test_lex_expect_tokens(str.data(), str.data() + str.size(), expected) &&
        test_lex_expect_tokens(std::basic_stringstream<CharT>(str), expected);
}

template <json::Utf8Char CharT>
static bool test_lex_expect_single_token(const std::basic_string<CharT>& str, json::lexer_token expected,
    std::basic_string_view<CharT> expectedStr)
{
    return test_lex_expect_single_token(str.data(), str.data() + str.size(), expected, expectedStr) &&
        test_lex_expect_single_token(std::basic_stringstream<CharT>(str), expected, expectedStr);
}

template <json::Utf8Char CharT>
static bool test_lex_expect_single_invalid(const std::basic_string<CharT>& str)
{
    return test_lex_expect_single_token(str, json::lexer_token::invalid, ""sv);
}

static int lex_null_test()
{
    test_guard guard{ "lex_null_test" };

    if (!test_lex_expect_single_token("null"s, json::lexer_token::keyword_null, "null"sv)) return 1;
    if (!test_lex_expect_single_token(" \t\r\nnull\n\r\t "s, json::lexer_token::keyword_null, "null"sv)) return 1;

    return guard.success();
}

static int lex_invalid_null_test()
{
    test_guard guard{ "lex_invalid_null_test" };

    if (!test_lex_expect_single_invalid("nul"s)) return 1;
    if (!test_lex_expect_single_invalid("nullnull"s)) return 1;

    return guard.success();
}

static int lex_true_test()
{
    test_guard guard{ "lex_true_test" };

    if (!test_lex_expect_single_token("true"s, json::lexer_token::keyword_true, "true"sv)) return 1;
    if (!test_lex_expect_single_token(" \t\r\ntrue\n\r\t "s, json::lexer_token::keyword_true, "true"sv)) return 1;

    return guard.success();
}

static int lex_invalid_true_test()
{
    test_guard guard{ "lex_invalid_true_test" };

    if (!test_lex_expect_single_invalid("tru"s)) return 1;
    if (!test_lex_expect_single_invalid("truetrue"s)) return 1;

    return guard.success();
}

static int lex_false_test()
{
    test_guard guard{ "lex_false_test" };

    if (!test_lex_expect_single_token("false"s, json::lexer_token::keyword_false, "false"sv)) return 1;
    if (!test_lex_expect_single_token(" \t\r\nfalse\n\r\t "s, json::lexer_token::keyword_false, "false"sv)) return 1;

    return guard.success();
}

static int lex_invalid_false_test()
{
    test_guard guard{ "lex_invalid_false_test" };

    if (!test_lex_expect_single_invalid("fals"s)) return 1;
    if (!test_lex_expect_single_invalid("falsefalse"s)) return 1;

    return guard.success();
}

static int lex_invalid_identifier_test()
{
    test_guard guard{ "lex_invalid_identifier_test" };

    if (!test_lex_expect_single_invalid("nothing"s)) return 1; // Should not cause issues with "null" handling
    if (!test_lex_expect_single_invalid("testing"s)) return 1; // Should not cause issues with "true" handling
    if (!test_lex_expect_single_invalid("forlorn"s)) return 1; // Should not cause issues with "false" handling
    if (!test_lex_expect_single_invalid("apple"s)) return 1; // No identifier starts with 'a'
    if (!test_lex_expect_single_invalid("_null"s)) return 1; // No identifier starts with '_'
    if (!test_lex_expect_single_invalid("("s)) return 1; // '(' is not valid JSON
    if (!test_lex_expect_single_invalid(";"s)) return 1; // ';' is not valid JSON
    if (!test_lex_expect_single_invalid("'"s)) return 1; // ''' is not valid JSON
    if (!test_lex_expect_single_invalid("."s)) return 1; // '.' is not valid JSON

    return guard.success();
}

static int lex_valid_number_test()
{
    test_guard guard{ "lex_valid_number_test" };

    if (!test_lex_expect_single_token("0"s, json::lexer_token::number, "0"sv)) return 1;
    if (!test_lex_expect_single_token("-0"s, json::lexer_token::number, "-0"sv)) return 1;
    if (!test_lex_expect_single_token("-0.0E-0"s, json::lexer_token::number, "-0.0E-0"sv)) return 1;
    if (!test_lex_expect_single_token("-0.0E+0"s, json::lexer_token::number, "-0.0E+0"sv)) return 1;
    if (!test_lex_expect_single_token("10.01e+01"s, json::lexer_token::number, "10.01e+01"sv)) return 1;
    if (!test_lex_expect_single_token(" \t\r\n10.01e+01\n\r\t "s, json::lexer_token::number, "10.01e+01"sv)) return 1;
    if (!test_lex_expect_single_token("42.42e42"s, json::lexer_token::number, "42.42e42"sv)) return 1;
    if (!test_lex_expect_single_token("-42.42e-42"s, json::lexer_token::number, "-42.42e-42"sv)) return 1;
    if (!test_lex_expect_single_token("1234567890.0987654321e1234567890"s, json::lexer_token::number,
            "1234567890.0987654321e1234567890"sv))
        return 1;

    return guard.success();
}

static int lex_invalid_number_test()
{
    test_guard guard{ "lex_invalid_number_test" };

    if (!test_lex_expect_single_invalid("042"s)) return 1;
    if (!test_lex_expect_single_invalid("+0"s)) return 1;
    if (!test_lex_expect_single_invalid("0-"s)) return 1;
    if (!test_lex_expect_single_invalid("+42"s)) return 1;
    if (!test_lex_expect_single_invalid("--42"s)) return 1;
    if (!test_lex_expect_single_invalid("-0-42"s)) return 1;
    if (!test_lex_expect_single_invalid("42e-+42"s)) return 1;
    if (!test_lex_expect_single_invalid("42e--42"s)) return 1;
    if (!test_lex_expect_single_invalid("42e+-42"s)) return 1;
    if (!test_lex_expect_single_invalid("42e++42"s)) return 1;
    if (!test_lex_expect_single_invalid("42.-42"s)) return 1;
    if (!test_lex_expect_single_invalid("42.+42"s)) return 1;
    if (!test_lex_expect_single_invalid(".42"s)) return 1;
    if (!test_lex_expect_single_invalid("42."s)) return 1;
    if (!test_lex_expect_single_invalid("42.e42"s)) return 1;
    if (!test_lex_expect_single_invalid("42.42.42"s)) return 1;
    if (!test_lex_expect_single_invalid("42e42e42"s)) return 1;
    if (!test_lex_expect_single_invalid("42.42e42.42"s)) return 1;

    return guard.success();
}

static int lex_invalid_text_test()
{
    test_guard guard{ "lex_invalid_text_test" };

    if (!test_lex_expect_single_invalid("foo"s)) return 1;
    if (!test_lex_expect_single_invalid("bar"s)) return 1;
    if (!test_lex_expect_single_invalid("testing true"s)) return 1;
    if (!test_lex_expect_single_invalid("nothing"s)) return 1;
    if (!test_lex_expect_single_invalid("unknown starting character"s)) return 1;

    return guard.success();
}

static int lex_valid_string_test()
{
    test_guard guard{ "lex_valid_string_test" };

    auto do_test = [](const std::u8string& str, std::u8string_view expected = {}) {
        if (expected.empty()) expected = str;
        auto data = u8"\""s + str + u8"\"";
        return test_lex_expect_single_token(data, json::lexer_token::string, expected) &&
            test_lex_expect_single_token(u8" \t\r\n" + data + u8"\n\r\t ", json::lexer_token::string, expected);
    };

    if (!do_test(u8"")) return 1;
    if (!do_test(u8"foo")) return 1;
    if (!do_test(u8"foo bar")) return 1;
    if (!do_test(u8"just a \\\"quoted\\\" string", u8"just a \"quoted\" string")) return 1;
    if (!do_test(u8"I \\u2665 unicode", u8"I \u2665 unicode")) return 1;
    if (!do_test(u8"I \u2665 unicode", u8"I \u2665 unicode")) return 1;
    if (!do_test(u8"\\uaBcD", u8"\uabcd")) return 1;
    if (!do_test(u8"\\\"\\\\\\/\\b\\f\\n\\r\\t", u8"\"\\/\b\f\n\r\t")) return 1;

    return guard.success();
}

static int lex_invalid_string_test()
{
    test_guard guard{ "lex_invalid_string_test" };

    if (!test_lex_expect_single_invalid("\""s)) return 1;
    if (!test_lex_expect_single_invalid("\"foo bar"s)) return 1;
    if (!test_lex_expect_single_invalid("\"foo bar\\\""s)) return 1;
    if (!test_lex_expect_single_invalid("\\\"foo bar\""s)) return 1;
    if (!test_lex_expect_single_invalid("\\\"\\"s)) return 1;
    if (!test_lex_expect_single_invalid("\"\\u 2665\""s)) return 1;
    if (!test_lex_expect_single_invalid("\"\\u266\""s)) return 1;
    if (!test_lex_expect_single_invalid("\"\\u266G\""s)) return 1;
    if (!test_lex_expect_single_invalid("\"\\x42\""s)) return 1;
    if (!test_lex_expect_single_invalid("\"\\q\""s)) return 1;
    if (!test_lex_expect_single_invalid("\"\v\""s)) return 1;

    return guard.success();
}

static int lex_array_test()
{
    test_guard guard{ "lex_array_test" };

    // This is intentionally a pretty simple test; any more would be testing things we test elsewhere
    if (!test_lex_expect_tokens("[]"s,
            { { json::lexer_token::bracket_open, "[" }, { json::lexer_token::bracket_close, "]" } }))
        return 1;

    if (!test_lex_expect_tokens("[ 42, true,0,null ]"s,
            { { json::lexer_token::bracket_open, "[" }, { json::lexer_token::number, "42" },
                { json::lexer_token::comma, "," }, { json::lexer_token::keyword_true, "true" },
                { json::lexer_token::comma, "," }, { json::lexer_token::number, "0" },
                { json::lexer_token::comma, "," }, { json::lexer_token::keyword_null, "null" },
                { json::lexer_token::bracket_close, "]" } }))
        return 1;

    return guard.success();
}

static int lex_object_test()
{
    test_guard guard{ "lex_object_test" };

    if (!test_lex_expect_tokens("{}"s,
            { { json::lexer_token::curly_open, "{" }, { json::lexer_token::curly_close, "}" } }))
        return 1;

    if (!test_lex_expect_tokens(R"^-^({ "answer": 42, "foo": "bar", "success": false })^-^"s,
            {
                { json::lexer_token::curly_open, "{" },
                { json::lexer_token::string, "answer" },
                { json::lexer_token::colon, ":" },
                { json::lexer_token::number, "42" },
                { json::lexer_token::comma, "," },
                { json::lexer_token::string, "foo" },
                { json::lexer_token::colon, ":" },
                { json::lexer_token::string, "bar" },
                { json::lexer_token::comma, "," },
                { json::lexer_token::string, "success" },
                { json::lexer_token::colon, ":" },
                { json::lexer_token::keyword_false, "false" },
                { json::lexer_token::curly_close, "}" },
            }))
        return 1;

    return guard.success();
}

int lexer_tests()
{
    int result = 0;
    result += lex_null_test();
    result += lex_invalid_null_test();
    result += lex_true_test();
    result += lex_invalid_true_test();
    result += lex_false_test();
    result += lex_invalid_false_test();
    result += lex_invalid_identifier_test();
    result += lex_valid_number_test();
    result += lex_invalid_number_test();
    result += lex_invalid_text_test();
    result += lex_valid_string_test();
    result += lex_invalid_string_test();
    result += lex_array_test();
    result += lex_object_test();
    return result;
}

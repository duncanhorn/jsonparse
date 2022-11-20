
#include <json_lexer.h>
#include <sstream>

#include "test_guard.h"

using namespace std::literals;

template <typename InputStream>
static bool test_lex_expect_tokens(json::lexer<InputStream>& lexer,
    std::initializer_list<std::pair<json::lexer_token, std::string_view>> expected)
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
                static_cast<int>(pair.second.size()), pair.second.data(), lexer.string_value.c_str());
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

template <typename InputStream>
static bool test_lex_expect_single_token(json::lexer<InputStream>& lexer, json::lexer_token expected,
    std::string_view expectedStr)
{
    return test_lex_expect_tokens(lexer, { { expected, expectedStr } });
}

static bool test_lex_expect_tokens(const char* begin, const char* end,
    std::initializer_list<std::pair<json::lexer_token, std::string_view>> expected)
{
    json::buffer_input_stream stream(begin, end);
    json::lexer lexer(stream);
    return test_lex_expect_tokens(lexer, expected);
}

static bool test_lex_expect_single_token(const char* begin, const char* end, json::lexer_token expected,
    std::string_view expectedStr)
{
    json::buffer_input_stream stream(begin, end);
    json::lexer lexer(stream);
    return test_lex_expect_single_token(lexer, expected, expectedStr);
}

static bool test_lex_expect_tokens(std::stringstream input,
    std::initializer_list<std::pair<json::lexer_token, std::string_view>> expected)
{
    json::istream stream(input);
    json::lexer lexer(stream);
    return test_lex_expect_tokens(lexer, expected);
}

static bool test_lex_expect_single_token(std::stringstream input, json::lexer_token expected,
    std::string_view expectedStr)
{
    json::istream stream(input);
    json::lexer lexer(stream);
    return test_lex_expect_single_token(lexer, expected, expectedStr);
}

static bool test_lex_expect_tokens(const std::string& str,
    std::initializer_list<std::pair<json::lexer_token, std::string_view>> expected)
{
    return test_lex_expect_tokens(str.data(), str.data() + str.size(), expected) &&
        test_lex_expect_tokens(std::stringstream(str), expected);
}

static bool test_lex_expect_single_token(const std::string& str, json::lexer_token expected,
    std::string_view expectedStr)
{
    return test_lex_expect_single_token(str.data(), str.data() + str.size(), expected, expectedStr) &&
        test_lex_expect_single_token(std::stringstream(str), expected, expectedStr);
}

static bool test_lex_expect_single_invalid(const std::string& str)
{
    return test_lex_expect_single_token(str, json::lexer_token::invalid, "");
}

static int lex_null_test()
{
    test_guard guard{ "lex_null_test" };

    if (!test_lex_expect_single_token("null", json::lexer_token::keyword_null, "null")) return 1;
    if (!test_lex_expect_single_token(" \t\r\nnull\n\r\t ", json::lexer_token::keyword_null, "null")) return 1;

    return guard.success();
}

static int lex_invalid_null_test()
{
    test_guard guard{ "lex_invalid_null_test" };

    if (!test_lex_expect_single_invalid("nul")) return 1;
    if (!test_lex_expect_single_invalid("nullnull")) return 1;

    return guard.success();
}

static int lex_true_test()
{
    test_guard guard{ "lex_true_test" };

    if (!test_lex_expect_single_token("true", json::lexer_token::keyword_true, "true")) return 1;
    if (!test_lex_expect_single_token(" \t\r\ntrue\n\r\t ", json::lexer_token::keyword_true, "true")) return 1;

    return guard.success();
}

static int lex_invalid_true_test()
{
    test_guard guard{ "lex_invalid_true_test" };

    if (!test_lex_expect_single_invalid("tru")) return 1;
    if (!test_lex_expect_single_invalid("truetrue")) return 1;

    return guard.success();
}

static int lex_false_test()
{
    test_guard guard{ "lex_false_test" };

    if (!test_lex_expect_single_token("false", json::lexer_token::keyword_false, "false")) return 1;
    if (!test_lex_expect_single_token(" \t\r\nfalse\n\r\t ", json::lexer_token::keyword_false, "false")) return 1;

    return guard.success();
}

static int lex_invalid_false_test()
{
    test_guard guard{ "lex_invalid_false_test" };

    if (!test_lex_expect_single_invalid("fals")) return 1;
    if (!test_lex_expect_single_invalid("falsefalse")) return 1;

    return guard.success();
}

static int lex_invalid_identifier_test()
{
    test_guard guard{ "lex_invalid_identifier_test" };

    if (!test_lex_expect_single_invalid("nothing")) return 1; // Should not cause issues with "null" handling
    if (!test_lex_expect_single_invalid("testing")) return 1; // Should not cause issues with "true" handling
    if (!test_lex_expect_single_invalid("forlorn")) return 1; // Should not cause issues with "false" handling
    if (!test_lex_expect_single_invalid("apple")) return 1; // No identifier starts with 'a'
    if (!test_lex_expect_single_invalid("_null")) return 1; // No identifier starts with '_'
    if (!test_lex_expect_single_invalid("(")) return 1; // '(' is not valid JSON
    if (!test_lex_expect_single_invalid(";")) return 1; // ';' is not valid JSON
    if (!test_lex_expect_single_invalid("'")) return 1; // ''' is not valid JSON
    if (!test_lex_expect_single_invalid(".")) return 1; // '.' is not valid JSON

    return guard.success();
}

static int lex_valid_number_test()
{
    test_guard guard{ "lex_valid_number_test" };

    if (!test_lex_expect_single_token("0", json::lexer_token::number, "0")) return 1;
    if (!test_lex_expect_single_token("-0", json::lexer_token::number, "-0")) return 1;
    if (!test_lex_expect_single_token("-0.0E-0", json::lexer_token::number, "-0.0E-0")) return 1;
    if (!test_lex_expect_single_token("-0.0E+0", json::lexer_token::number, "-0.0E+0")) return 1;
    if (!test_lex_expect_single_token("10.01e+01", json::lexer_token::number, "10.01e+01")) return 1;
    if (!test_lex_expect_single_token(" \t\r\n10.01e+01\n\r\t ", json::lexer_token::number, "10.01e+01")) return 1;
    if (!test_lex_expect_single_token("42.42e42", json::lexer_token::number, "42.42e42")) return 1;
    if (!test_lex_expect_single_token("-42.42e-42", json::lexer_token::number, "-42.42e-42")) return 1;
    if (!test_lex_expect_single_token("1234567890.0987654321e1234567890", json::lexer_token::number,
            "1234567890.0987654321e1234567890"))
        return 1;

    return guard.success();
}

static int lex_invalid_number_test()
{
    test_guard guard{ "lex_invalid_number_test" };

    if (!test_lex_expect_single_invalid("042")) return 1;
    if (!test_lex_expect_single_invalid("+0")) return 1;
    if (!test_lex_expect_single_invalid("0-")) return 1;
    if (!test_lex_expect_single_invalid("+42")) return 1;
    if (!test_lex_expect_single_invalid("--42")) return 1;
    if (!test_lex_expect_single_invalid("-0-42")) return 1;
    if (!test_lex_expect_single_invalid("42e-+42")) return 1;
    if (!test_lex_expect_single_invalid("42e--42")) return 1;
    if (!test_lex_expect_single_invalid("42e+-42")) return 1;
    if (!test_lex_expect_single_invalid("42e++42")) return 1;
    if (!test_lex_expect_single_invalid("42.-42")) return 1;
    if (!test_lex_expect_single_invalid("42.+42")) return 1;
    if (!test_lex_expect_single_invalid(".42")) return 1;
    if (!test_lex_expect_single_invalid("42.")) return 1;
    if (!test_lex_expect_single_invalid("42.e42")) return 1;
    if (!test_lex_expect_single_invalid("42.42.42")) return 1;
    if (!test_lex_expect_single_invalid("42e42e42")) return 1;
    if (!test_lex_expect_single_invalid("42.42e42.42")) return 1;

    return guard.success();
}

static int lex_invalid_text_test()
{
    test_guard guard{ "lex_invalid_text_test" };

    if (!test_lex_expect_single_invalid("foo")) return 1;
    if (!test_lex_expect_single_invalid("bar")) return 1;
    if (!test_lex_expect_single_invalid("testing true")) return 1;
    if (!test_lex_expect_single_invalid("nothing")) return 1;
    if (!test_lex_expect_single_invalid("unknown starting character")) return 1;

    return guard.success();
}

static int lex_valid_string_test()
{
    test_guard guard{ "lex_valid_string_test" };

    auto do_test = [](auto& str, const char* expected = nullptr) {
        if (!expected) expected = str;
        auto data = "\""s + str + "\"";
        return test_lex_expect_single_token(data, json::lexer_token::string, expected) &&
            test_lex_expect_single_token(" \t\r\n" + data + "\n\r\t ", json::lexer_token::string, expected);
    };

    if (!do_test("")) return 1;
    if (!do_test("foo")) return 1;
    if (!do_test("foo bar")) return 1;
    if (!do_test("just a \\\"quoted\\\" string", "just a \"quoted\" string")) return 1;
    if (!do_test("I \\u2665 unicode", "I \u2665 unicode")) return 1;
    if (!do_test("I \u2665 unicode", "I \u2665 unicode")) return 1;
    if (!do_test("\\uaBcD", "\uabcd")) return 1;
    if (!do_test("\\\"\\\\\\/\\b\\f\\n\\r\\t", "\"\\/\b\f\n\r\t")) return 1;

    return guard.success();
}

static int lex_invalid_string_test()
{
    test_guard guard{ "lex_invalid_string_test" };

    if (!test_lex_expect_single_invalid("\"")) return 1;
    if (!test_lex_expect_single_invalid("\"foo bar")) return 1;
    if (!test_lex_expect_single_invalid("\"foo bar\\\"")) return 1;
    if (!test_lex_expect_single_invalid("\\\"foo bar\"")) return 1;
    if (!test_lex_expect_single_invalid("\\\"\\")) return 1;
    if (!test_lex_expect_single_invalid("\"\\u 2665\"")) return 1;
    if (!test_lex_expect_single_invalid("\"\\u266\"")) return 1;
    if (!test_lex_expect_single_invalid("\"\\u266G\"")) return 1;
    if (!test_lex_expect_single_invalid("\"\\x42\"")) return 1;
    if (!test_lex_expect_single_invalid("\"\\q\"")) return 1;
    if (!test_lex_expect_single_invalid("\"\v\"")) return 1;

    return guard.success();
}

static int lex_array_test()
{
    test_guard guard{ "lex_array_test" };

    // This is intentionally a pretty simple test; any more would be testing things we test elsewhere
    if (!test_lex_expect_tokens("[]",
            { { json::lexer_token::bracket_open, "[" }, { json::lexer_token::bracket_close, "]" } }))
        return 1;

    if (!test_lex_expect_tokens("[ 42, true,0,null ]",
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

    if (!test_lex_expect_tokens("{}",
            { { json::lexer_token::curly_open, "{" }, { json::lexer_token::curly_close, "}" } }))
        return 1;

    if (!test_lex_expect_tokens(R"^-^({ "answer": 42, "foo": "bar", "success": false })^-^",
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

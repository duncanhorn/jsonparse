
#include <json_lexer.h>

#include "test_guard.h"

static bool lex_expect_single_token(const char* begin, const char* end, json::lexer_token expected)
{
    json::buffer_input_stream stream(begin, end);
    json::lexer lexer(stream);

    if (lexer.current_token != expected)
    {
        std::printf("ERROR: Incorrect token\n");
        return false;
    }

    lexer.advance();
    if (lexer.current_token != json::lexer_token::eof)
    {
        std::printf("ERROR: Incorrect token; expected eof\n");
        return false;
    }

    return true;
}

static int lex_null_test()
{
    test_guard guard{ "lex_null_test" };

    const char text[] = " \t\r\nnull \t\r\n";
    if (!lex_expect_single_token(text, text + std::size(text) - 1, json::lexer_token::keyword_null)) return 1;

    return guard.success();
}

static int lex_true_test()
{
    test_guard guard{ "lex_true_test" };

    const char text[] = " \t\r\ntrue \t\r\n";
    if (!lex_expect_single_token(text, text + std::size(text) - 1, json::lexer_token::keyword_true)) return 1;

    return guard.success();
}

static int lex_false_test()
{
    test_guard guard{ "lex_false_test" };

    const char text[] = " \t\r\nfalse \t\r\n";
    if (!lex_expect_single_token(text, text + std::size(text) - 1, json::lexer_token::keyword_false)) return 1;

    return guard.success();
}

static int lex_valid_number_test()
{
    test_guard guard{ "lex_valid_number_test" };

    auto do_test = [](auto& str)
    {
        std::string buffer = " \t\r\n";
        buffer += str;
        buffer += " \t\r\n";
        json::buffer_input_stream stream(buffer.c_str(), buffer.c_str() + buffer.size());
        json::lexer lexer(stream);

        if (lexer.current_token != json::lexer_token::number)
        {
            std::printf("ERROR: Incorrect token; expected number\n");
            return false;
        }
        else if (lexer.string_value != str)
        {
            std::printf("ERROR: Text did not match; expected '%s', got '%s'\n", str, lexer.string_value.c_str());
            return false;
        }

        lexer.advance();
        if (lexer.current_token != json::lexer_token::eof)
        {
            std::printf("ERROR: Incorrect token; expected eof\n");
            return false;
        }

        return true;
    };

    if (!do_test("0")) return 1;
    if (!do_test("-0")) return 1;
    if (!do_test("-0.0E-0")) return 1;
    if (!do_test("-0.0E+0")) return 1;
    if (!do_test("10.01e+01")) return 1;
    if (!do_test("42.42e42")) return 1;

    return guard.success();
}

static int lex_invalid_number_test()
{
    test_guard guard{ "lex_invalid_number_test" };

    auto do_test = [](auto& str)
    {
        json::buffer_input_stream stream(str, str + std::size(str));
        json::lexer lexer(stream);

        if (lexer.current_token != json::lexer_token::invalid)
        {
            std::printf("ERROR: Incorrect token; expected invalid\n");
            return false;
        }
        return true;
    };

    if (!do_test("+0")) return 1;
    if (!do_test("+42")) return 1;
    if (!do_test(".42")) return 1;
    if (!do_test("42.")) return 1;

    return guard.success();
}

static int lex_valid_string_test()
{
    test_guard guard{ "lex_valid_string_test" };

    auto do_test = [](auto& str, const char* expected = nullptr)
    {
        if (!expected) expected = str;

        std::string buffer = " \t\r\n\"";
        buffer += str;
        buffer += "\" \t\r\n";
        json::buffer_input_stream stream(buffer.c_str(), buffer.c_str() + buffer.size());
        json::lexer lexer(stream);

        if (lexer.current_token != json::lexer_token::string)
        {
            std::printf("ERROR: Incorrect token; expected string\n");
            return false;
        }
        else if (lexer.string_value != expected)
        {
            std::printf("ERROR: Text did not match; expected '%s', got '%s'\n", expected, lexer.string_value.c_str());
            return false;
        }

        lexer.advance();
        if (lexer.current_token != json::lexer_token::eof)
        {
            std::printf("ERROR: Incorrect token; expected eof\n");
            return false;
        }

        return true;
    };

    if (!do_test("")) return 1;
    if (!do_test("foo")) return 1;
    if (!do_test("foo bar")) return 1;
    if (!do_test("just a \\\"quoted\\\" string", "just a \"quoted\" string")) return 1;
    if (!do_test("I \\u2665 unicode", "I \u2665 unicode")) return 1;
    if (!do_test(u8"I ♥ unicode", u8"I \u2665 unicode")) return 1;
    if (!do_test("\\\"\\\\\\/\\b\\f\\n\\r\\t", "\"\\/\b\f\n\r\t")) return 1;

    return guard.success();
}

static int lex_invalid_string_test()
{
    test_guard guard{ "lex_invalid_string_test" };

    auto do_test = [](auto& str)
    {
        json::buffer_input_stream stream(str, str + std::size(str) - 1);
        json::lexer lexer(stream);

        if (lexer.current_token != json::lexer_token::invalid)
        {
            std::printf("ERROR: Incorrect token; expected invalid\n");
            return false;
        }
        return true;
    };

    if (!do_test("\"")) return 1;
    if (!do_test("\"foo bar")) return 1;
    if (!do_test("\"foo bar\\\"")) return 1;
    if (!do_test("\"\\uFFEX\"")) return 1;
    if (!do_test("\"\\uFFE\"")) return 1;
    if (!do_test("\"\\x42\"")) return 1;
    if (!do_test("\"\v\"")) return 1;

    return guard.success();
}

static int lex_array_test()
{
    test_guard guard{ "lex_array_test" };

    auto do_test = [](auto& str, auto&& callback)
    {
        std::string buffer = " \t\r\n";
        buffer += str;
        buffer += " \t\r\n";
        json::buffer_input_stream stream(buffer.c_str(), buffer.c_str() + buffer.size());
        json::lexer lexer(stream);
        return callback(lexer);
    };

    // This is intentionally a pretty simple test; any more would be testing things we test elsewhere
    if (!do_test("[]", [](json::lexer<json::buffer_input_stream>& lexer)
    {
        if (lexer.current_token != json::lexer_token::bracket_open)
        {
            std::printf("ERROR: Incorrect token; expected '['\n");
            return false;
        }

        lexer.advance();
        if (lexer.current_token != json::lexer_token::bracket_close)
        {
            std::printf("ERROR: Incorrect token; expected ']'\n");
            return false;
        }

        lexer.advance();
        if (lexer.current_token != json::lexer_token::eof)
        {
            std::printf("ERROR: Incorrect token; expected eof\n");
            return false;
        }

        return true;
    })) return 1;

    if (!do_test("[ 42, true,0,null ]", [](json::lexer<json::buffer_input_stream>& lexer)
    {
        if (lexer.current_token != json::lexer_token::bracket_open)
        {
            std::printf("ERROR: Incorrect token; expected '['\n");
            return false;
        }

        lexer.advance(); // Should now be '42'
        lexer.advance();
        if (lexer.current_token != json::lexer_token::comma)
        {
            std::printf("ERROR: Incorrect token; expected ','\n");
            return false;
        }

        lexer.advance(); // Should now be 'true'
        lexer.advance();
        if (lexer.current_token != json::lexer_token::comma)
        {
            std::printf("ERROR: Incorrect token; expected ']'\n");
            return false;
        }

        lexer.advance(); // Should now be '0'
        lexer.advance();
        if (lexer.current_token != json::lexer_token::comma)
        {
            std::printf("ERROR: Incorrect token; expected ','\n");
            return false;
        }

        lexer.advance(); // Should now be 'null'
        lexer.advance();
        if (lexer.current_token != json::lexer_token::bracket_close)
        {
            std::printf("ERROR: Incorrect token; expected ']'\n");
            return false;
        }

        lexer.advance();
        if (lexer.current_token != json::lexer_token::eof)
        {
            std::printf("ERROR: Incorrect token; expected eof\n");
            return false;
        }

        return true;
    })) return 1;

    return guard.success();
}

static int lex_object_test()
{
    test_guard guard{ "lex_object_test" };

    auto do_test = [](auto& str, auto&& callback)
    {
        std::string buffer = " \t\r\n";
        buffer += str;
        buffer += " \t\r\n";
        json::buffer_input_stream stream(buffer.c_str(), buffer.c_str() + buffer.size());
        json::lexer lexer(stream);
        return callback(lexer);
    };

    if (!do_test("{}", [](json::lexer<json::buffer_input_stream>& lexer)
    {
        if (lexer.current_token != json::lexer_token::curly_open)
        {
            std::printf("ERROR: Incorrect token; expected '{'\n");
            return false;
        }

        lexer.advance();
        if (lexer.current_token != json::lexer_token::curly_close)
        {
            std::printf("ERROR: Incorrect token; expected '}'\n");
            return false;
        }

        return true;
    })) return 1;

    if (!do_test("{ \"answer\": 42, \"foo\": \"bar\", \"success\": false }", [](json::lexer<json::buffer_input_stream>& lexer)
    {
        if (lexer.current_token != json::lexer_token::curly_open)
        {
            std::printf("ERROR: Incorrect token; expected '{'\n");
            return false;
        }

        lexer.advance(); // "answer"
        lexer.advance();
        if (lexer.current_token != json::lexer_token::colon)
        {
            std::printf("ERROR: Incorrect token; expected ':'\n");
            return false;
        }

        lexer.advance(); // 42
        lexer.advance();
        if (lexer.current_token != json::lexer_token::comma)
        {
            std::printf("ERROR: Incorrect token; expected ','\n");
            return false;
        }

        lexer.advance(); // "foo"
        lexer.advance();
        if (lexer.current_token != json::lexer_token::colon)
        {
            std::printf("ERROR: Incorrect token; expected ':'\n");
            return false;
        }

        lexer.advance(); // "bar"
        lexer.advance();
        if (lexer.current_token != json::lexer_token::comma)
        {
            std::printf("ERROR: Incorrect token; expected ','\n");
            return false;
        }

        lexer.advance(); // "success"
        lexer.advance();
        if (lexer.current_token != json::lexer_token::colon)
        {
            std::printf("ERROR: Incorrect token; expected ':'\n");
            return false;
        }

        lexer.advance(); // false
        lexer.advance();
        if (lexer.current_token != json::lexer_token::curly_close)
        {
            std::printf("ERROR: Incorrect token; expected '}'\n");
            return false;
        }

        return true;
    })) return 1;

    return guard.success();
}

int lexer_tests()
{
    int result = 0;
    result += lex_null_test();
    result += lex_true_test();
    result += lex_false_test();
    result += lex_valid_number_test();
    result += lex_invalid_number_test();
    result += lex_valid_string_test();
    result += lex_invalid_string_test();
    result += lex_array_test();
    result += lex_object_test();
    return result;
}

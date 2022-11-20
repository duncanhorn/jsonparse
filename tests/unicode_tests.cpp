
#include <json_lexer.h>

#include "test_guard.h"

static int utf8_read_valid_test()
{
    test_guard guard{ "utf8_read_valid_test" };

    auto do_test = [](auto& str, char32_t expect) {
        auto begin = str;
        auto end = begin + std::size(str) - 1; // -1 for null
        auto [ch, ptr] = json::utf8_read(begin, end);
        if (ptr != end)
        {
            std::printf("ERROR: Failed to read character\n");
            return false;
        }
        else if (ch != expect)
        {
            std::printf("ERROR: Incorrect character read. Expected %d, got %d\n", expect, ch);
            return false;
        }
        return true;
    };

    if (!do_test("\u0000", U'\u0000')) return 1;
    if (!do_test("\u007F", U'\u007F')) return 1;
    if (!do_test("\u0080", U'\u0080')) return 1;
    if (!do_test("\u07FF", U'\u07FF')) return 1;
    if (!do_test("\u0800", U'\u0800')) return 1;
    if (!do_test("\uFFFF", U'\uFFFF')) return 1;
    if (!do_test("\U00010000", U'\U00010000')) return 1;
    if (!do_test("\U0010FFFF", U'\U0010FFFF')) return 1;
    return guard.success();
}

static int utf8_read_invalid_test()
{
    test_guard guard{ "utf8_read_invalid_test" };

    auto do_test = [](char value) {
        // NOTE: The expectation is that all input is > 0x7F
        auto begin = &value;
        auto [ch, ptr] = json::utf8_read(begin, begin + 1);
        if (ptr != &value)
        {
            std::printf("ERROR: Advanced pointer for too-small buffer\n");
        }

        return true;
    };
    if (!do_test(0xC0)) return 1;
    if (!do_test(0xE0)) return 1;
    if (!do_test(0xF0)) return 1;

    auto do_invalid = [](char value) {
        // NOTE: The expectation is that all input are invalid leading bytes
        auto begin = &value;
        auto [ch, ptr] = json::utf8_read(begin, begin + 42);
        if (ptr != &value)
        {
            std::printf("ERROR: Advanced pointer for invalid input\n");
        }

        return true;
    };
    do_invalid(0x80);
    do_invalid(0xF8);

    return guard.success();
}

static int utf8_append_test()
{
    test_guard guard{ "utf8_append_test" };

    const char testString[] = "\u0000\u007F\u0080\u07FF\u0800\uFFFF\U00010000\U0010FFFF";
    auto begin = testString;
    auto end = testString + std::size(testString) - 1; // -1 for null
    std::string str;
    while (begin != end)
    {
        auto [ch, ptr] = json::utf8_read(begin, end);
        if (ptr == begin)
        {
            std::printf("ERROR: Failed to advance read character\n");
            return 1;
        }

        begin = ptr;
        if (!json::utf8_append(str, ch))
        {
            std::printf("ERROR: Failed to write character\n");
            return 1;
        }
    }

    if (str != std::string_view(testString, std::size(testString) - 1))
    {
        std::printf("ERROR: Copied string incorrectly\n");
        return 1;
    }

    return guard.success();
}

int unicode_tests()
{
    auto result = 0;
    result += utf8_read_valid_test();
    result += utf8_read_invalid_test();
    result += utf8_append_test();
    return result;
}

#pragma once

#include <cassert>
#include <istream>
#include <string_view>

namespace json
{
    constexpr char invalid_char = 0xFF;

    constexpr std::size_t utf8_code_unit_read_size(char ch) noexcept
    {
        return ((ch & 0x80) == 0x00) ? 1 :
            ((ch & 0xE0) == 0xC0)    ? 2 :
            ((ch & 0xF0) == 0xE0)    ? 3 :
            ((ch & 0xF8) == 0xF0)    ? 4 :
                                       0;
    }

    constexpr std::pair<char32_t, const char*> utf8_read(const char* begin, const char* end) noexcept
    {
        assert(begin != end);
        auto size = utf8_code_unit_read_size(*begin);
        if (!size) return { 0, begin };
        if (static_cast<std::size_t>(end - begin) < size) return { 0, begin };

        char initialMasks[] = { 0x00, 0x7F, 0x1F, 0x0F, 0x07 };
        char32_t result = (*begin++) & initialMasks[size];

        switch (size)
        {
        case 4: result = (result << 6) | ((*begin++) & 0x3F); [[fallthrough]];
        case 3: result = (result << 6) | ((*begin++) & 0x3F); [[fallthrough]];
        case 2: result = (result << 6) | ((*begin++) & 0x3F);
        }

        return { result, begin };
    }

    constexpr std::size_t utf8_code_unit_write_size(char32_t ch) noexcept
    {
        return (ch < 0x0080) ? 1 : (ch < 0x0800) ? 2 : (ch < 0x10000) ? 3 : (ch < 0x110000) ? 4 : 0;
    }

    inline bool utf8_append(std::string& target, char32_t ch)
    {
        auto size = utf8_code_unit_write_size(ch);
        if (!size) return false;

        char32_t initialMasks[] = { 0x00, 0x7F, 0x1F, 0x0F, 0x07 };
        char32_t initialMarks[] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };
        std::size_t initialShifts[] = { 0, 0, 6, 12, 18 }; // 6 * (size - 1)
        target.push_back(static_cast<char>(((ch >> initialShifts[size]) & initialMasks[size]) | initialMarks[size]));

        switch (size)
        {
        case 4: target.push_back(static_cast<char>(((ch >> 12) & 0x3F) | 0x80));
        case 3: target.push_back(static_cast<char>(((ch >> 6) & 0x3F) | 0x80));
        case 2: target.push_back(static_cast<char>(((ch >> 0) & 0x3F) | 0x80));
        }

        return true;
    }

    namespace details
    {
        constexpr bool in_range(char32_t ch, char32_t begin, char32_t end) noexcept
        {
            return (ch >= begin) && (ch <= end);
        }
    }

    constexpr bool is_whitespace(char32_t ch) noexcept
    {
        return (ch == ' ') || (ch == '\n') || (ch == '\r') || (ch == '\t');
    }

    constexpr bool is_digit(char32_t ch) noexcept
    {
        return details::in_range(ch, '0', '9');
    }

    constexpr bool is_hex_digit(char32_t ch) noexcept
    {
        return is_digit(ch) || details::in_range(ch, 'a', 'f') || details::in_range(ch, 'A', 'F');
    }

    template <typename T>
    concept InputStream = requires(T value) {
                              {
                                  value.operator bool()
                                  } -> std::same_as<bool>;
                          };

    struct buffer_input_stream
    {
        const char* read;
        const char* end;

        buffer_input_stream(const char* begin, const char* end) noexcept : read(begin), end(end) {}
        buffer_input_stream(std::string_view str) noexcept : buffer_input_stream(str.data(), str.data() + str.size()) {}

        constexpr operator bool() const noexcept
        {
            return read != end;
        }

        constexpr bool eof() const noexcept
        {
            return read == end;
        }

        constexpr char get() noexcept
        {
            return eof() ? invalid_char : *read++;
        }

        constexpr char peek() noexcept
        {
            return eof() ? invalid_char : *read;
        }
    };

    struct istream
    {
        std::istream& stream;

        istream(std::istream& stream) noexcept : stream(stream) {}

        operator bool() const noexcept
        {
            return stream.good();
        }

        constexpr bool eof() const noexcept
        {
            return stream.eof();
        }

        constexpr char get() noexcept
        {
            return static_cast<char>(stream.get());
        }

        constexpr char peek() noexcept
        {
            return static_cast<char>(stream.peek());
        }
    };

    enum class lexer_token
    {
        // State/status tokens
        invalid,
        eof,

        // Characters
        curly_open, // {
        curly_close, // }
        bracket_open, // [
        bracket_close, // ]
        comma, // ,
        colon, // :

        // Keywords
        keyword_true,
        keyword_false,
        keyword_null,

        // Arbitrary length values
        string,
        number,
    };

    template <typename InputStream>
    struct lexer
    {
        InputStream& input;
        lexer_token current_token = lexer_token::eof; // Since we hard stop on invalid; 'advance' will work correctly
        std::string string_value;
        const char* error_text = nullptr;

        lexer(InputStream& input) : input(input)
        {
            advance();
        }

        void advance()
        {
            if (current_token == lexer_token::invalid) return;

            error_text = nullptr;
            string_value.clear();

            skip_whitespace();
            if (input.eof())
            {
                current_token = lexer_token::eof;
                return;
            }

            current_token = lexer_token::invalid;
            if (!input)
            {
                error_text = "Bad unicode";
                return;
            }

            auto ch = input.get();
            switch (ch)
            {
            case '{':
                current_token = lexer_token::curly_open;
                string_value = "{";
                break;

            case '}':
                current_token = lexer_token::curly_close;
                string_value = "}";
                break;

            case '[':
                current_token = lexer_token::bracket_open;
                string_value = "[";
                break;

            case ']':
                current_token = lexer_token::bracket_close;
                string_value = "]";
                break;

            case ',':
                current_token = lexer_token::comma;
                string_value = ",";
                break;

            case ':':
                current_token = lexer_token::colon;
                string_value = ":";
                break;

            case '"': process_string(); break;

            case 'n':
                // Either 'null' or error
                if (try_consume_remaining_string("ull"))
                {
                    current_token = lexer_token::keyword_null;
                    string_value = "null";
                }
                break;

            case 't':
                // Either 'true' or error
                if (try_consume_remaining_string("rue"))
                {
                    current_token = lexer_token::keyword_true;
                    string_value = "true";
                }
                break;

            case 'f':
                // Either 'false' or error
                if (try_consume_remaining_string("alse"))
                {
                    current_token = lexer_token::keyword_false;
                    string_value = "false";
                }
                break;

            default:
                // Must be a number
                process_number(ch);
                break;
            }
        }

    private:
        void skip_whitespace()
        {
            while (is_whitespace(input.peek()))
            {
                input.get();
            }
        }

        void process_string()
        {
            // NOTE: The leading '"' character should already be "consumed"
            while (true)
            {
                auto ch = input.get();
                if (ch == invalid_char)
                {
                    string_value.clear();
                    error_text = input.eof() ? "Unterminated string" : "Bad unicode";
                    return;
                }
                else if (ch == '"')
                {
                    // End of the string
                    break;
                }
                else if (ch == '\\')
                {
                    ch = input.get();
                    switch (ch)
                    {
                    case '"':
                    case '\\':
                    case '/': string_value.push_back(static_cast<char>(ch)); break;
                    case 'b': string_value.push_back('\b'); break;
                    case 'f': string_value.push_back('\f'); break;
                    case 'n': string_value.push_back('\n'); break;
                    case 'r': string_value.push_back('\r'); break;
                    case 't': string_value.push_back('\t'); break;
                    case 'u': {
                        char32_t decodedValue = 0;
                        for (std::size_t i = 0; i < 4; ++i)
                        {
                            decodedValue <<= 4;
                            ch = input.get();
                            if (is_digit(ch))
                            {
                                decodedValue |= (ch - '0');
                            }
                            else if (details::in_range(ch, 'a', 'f'))
                            {
                                decodedValue |= 10 + (ch - 'a');
                            }
                            else if (details::in_range(ch, 'A', 'F'))
                            {
                                decodedValue |= 10 + (ch - 'A');
                            }
                            else
                            {
                                string_value.clear();
                                error_text = input.eof() ? "Unterminated string" : "Bad unicode";
                                return;
                            }
                        }

                        // NOTE: Can't fail since max of 4 hex digits
                        utf8_append(string_value, decodedValue);
                    }
                    break;

                    default:
                        string_value.clear();
                        error_text = input.eof() ? "Unterminated string" :
                            input                ? "Unknown escape character" :
                                                   "Bad unicode";
                        return;
                    }
                }
                else if (static_cast<unsigned char>(ch) < 0x20)
                {
                    string_value.clear();
                    error_text = "Control character in string";
                    return;
                }
                else
                {
                    string_value.push_back(ch);
                }
            }

            // NOTE: Currently we handle something like '"foo""bar"' as two different strings. It might be worth it to
            // threat this as an error

            current_token = lexer_token::string;
        }

        void process_number(char ch)
        {
            if (ch == '-')
            {
                string_value.push_back('-');
                ch = input.get();
            }

            // Must at least have a leading zero
            if (!is_digit(ch))
            {
                string_value.clear();
                error_text = "Unknown value";
                return;
            }

            string_value.push_back(static_cast<char>(ch));
            if (ch != '0')
            {
                while (is_digit(input.peek()))
                {
                    string_value.push_back(static_cast<char>(input.get()));
                }
            }

            if (input.peek() == '.')
            {
                input.get();
                string_value.push_back('.');

                if (!is_digit(input.peek()))
                {
                    string_value.clear();
                    error_text = "Invalid number";
                    return;
                }

                while (is_digit(input.peek()))
                {
                    string_value.push_back(static_cast<char>(input.get()));
                }
            }

            if ((input.peek() == 'e') || (input.peek() == 'E'))
            {
                string_value.push_back(static_cast<char>(input.get()));

                if ((input.peek() == '-') || (input.peek() == '+'))
                {
                    string_value.push_back(static_cast<char>(input.get()));
                }

                if (!is_digit(input.peek()))
                {
                    string_value.clear();
                    error_text = "Invalid number";
                    return;
                }

                while (is_digit(input.peek()))
                {
                    string_value.push_back(static_cast<char>(input.get()));
                }
            }

            if (!next_is_separating_character())
            {
                string_value.clear();
                error_text = "Invalid number";
                return;
            }

            // TODO: Validate? Or just rely on future error on bad following characters?
            current_token = lexer_token::number;
        }

        bool try_consume_remaining_string(std::string_view remainder)
        {
            for (auto ch : remainder)
            {
                if (ch != input.get())
                {
                    error_text = "Unknown value";
                    return false;
                }
            }

            // We still need to validate that this was not just a substring.
            if (!next_is_separating_character())
            {
                error_text = "Unknown value";
                return false;
            }

            return true;
        }

        bool next_is_separating_character()
        {
            auto ch = input.peek();
            if (input.eof()) return true;

            // There's no true "great" definition of what defines a JSON lexer token. E.g. the string "truefalse" should
            // obviously not be treated as two separate tokens ("true" followed by "false"), however the handling of
            // something like "true&&false" is less cut and dry. Obviously, this is bad JSON input, however *where* the
            // failure occurs is not defined. To make things simpler (and therefore faster), we'll separate at
            // whitespace or any other non-word/number token
            static constexpr const std::string_view separators = " \n\r\t{}[],:\"";
            return separators.find_first_of(ch) != std::string_view::npos;
        }
    };
}

#pragma once

#include <charconv>
#include <system_error>

#include "json_lexer.h"

namespace json
{
    namespace details
    {
        template <typename InputStream> null_t parse_null(lexer<InputStream>& lex);
        template <typename InputStream> boolean_t parse_boolean(lexer<InputStream>& lex);
        template <typename InputStream> number_t parse_number(lexer<InputStream>& lex);
        template <typename InputStream> string_t parse_string(lexer<InputStream>& lex);
        template <typename InputStream> array_t<> parse_array(lexer<InputStream>& lex);
        template <typename InputStream> object_t parse_object(lexer<InputStream>& lex);

        template <typename InputStream>
        value parse(lexer<InputStream>& lex)
        {
            switch (lex.current_token)
            {
            case lexer_token::keyword_null: return parse_null(lex);
            case lexer_token::keyword_true:
            case lexer_token::keyword_false: return parse_boolean(lex);
            case lexer_token::number: return parse_number(lex);
            case lexer_token::string: return parse_string(lex);
            case lexer_token::bracket_open: return parse_array(lex);
            case lexer_token::curly_open: return parse_object(lex);
            default: break;
            }

            throw std::runtime_error("Invalid JSON; expected a value, but received '" + lex.string_value + "'");
        }

        template <typename InputStream>
        null_t parse_null(lexer<InputStream>& lex)
        {
            if (lex.current_token != lexer_token::keyword_null)
            {
                throw std::runtime_error("Invalid JSON; expected 'null', but received '" + lex.string_value + "'");
            }

            lex.advance();
            return nullptr;
        }

        template <typename InputStream>
        boolean_t parse_boolean(lexer<InputStream>& lex)
        {
            if (lex.current_token == lexer_token::keyword_true)
            {
                lex.advance();
                return true;
            }
            else if (lex.current_token == lexer_token::keyword_false)
            {
                lex.advance();
                return false;
            }

            throw std::runtime_error("Invalid JSON; expected boolean, but received '" + lex.string_value + "'");
        }

        template <typename InputStream>
        number_t parse_number(lexer<InputStream>& lex)
        {
            if (lex.current_token != lexer_token::number)
            {
                throw std::runtime_error("Invalid JSON; expected number, but received '" + lex.string_value + "'");
            }

            number_t result;
            auto begin = lex.string_value.c_str();
            auto end = begin + lex.string_value.size();
            auto [ptr, ec] = std::from_chars(begin, end, result);
            if (ec != std::errc{})
            {
                throw std::system_error(std::make_error_code(ec), "Failed to parse number '" + lex.string_value + "'");
            }
            else if (ptr != end)
            {
                throw std::runtime_error("Failed to parse number '" + lex.string_value + "'");
            }

            lex.advance();
            return result;
        }

        template <typename InputStream>
        string_t parse_string(lexer<InputStream>& lex)
        {
            if (lex.current_token != lexer_token::string)
            {
                throw std::runtime_error("Invalid JSON; expected string, but received '" + lex.string_value + "'");
            }

            string_t result = std::move(lex.string_value);
            lex.advance();
            return result;
        }

        template <typename InputStream>
        array_t<> parse_array(lexer<InputStream>& lex)
        {
            if (lex.current_token != lexer_token::bracket_open)
            {
                throw std::runtime_error("Invalid JSON; expected '[', but received '" + lex.string_value + "'");
            }

            array_t<> result;
            lex.advance();
            if (lex.current_token == lexer_token::bracket_close)
            {
                lex.advance();
                return result; // Empty array
            }

            while (true)
            {
                result.push_back(parse(lex));

                if (lex.current_token == lexer_token::bracket_close) break;
                else if (lex.current_token == lexer_token::comma)
                {
                    lex.advance();
                }
                else
                {
                    throw std::runtime_error("Invalid JSON array; expected ',', but received '" + lex.string_value + "'");
                }
            }

            lex.advance();
            return result;
        }

        template <typename InputStream>
        object_t parse_object(lexer<InputStream>& lex)
        {
            if (lex.current_token != lexer_token::curly_open)
            {
                throw std::runtime_error("Invalid JSON; expected '{', but received '" + lex.string_value + "'");
            }

            object_t result;
            lex.advance();
            if (lex.current_token == lexer_token::curly_close)
            {
                lex.advance();
                return result; // Empty object
            }

            while (true)
            {
                auto key = parse_string(lex);
                if (lex.current_token != lexer_token::colon)
                {
                    throw std::runtime_error("Invalid JSON object; expected ':', but received '" + lex.string_value + "'");
                }

                lex.advance();
                auto [itr, added] = result.emplace(std::move(key), parse(lex));
                if (!added)
                {
                    throw std::runtime_error("Invalid JSON object; duplicate member name '" + itr->first + "'");
                }

                if (lex.current_token == lexer_token::curly_close) break;
                else if (lex.current_token == lexer_token::comma)
                {
                    lex.advance();
                }
                else
                {
                    throw std::runtime_error("Invalid JSON object; expected ',', but received '" + lex.string_value + "'");
                }
            }

            lex.advance();
            return result;
        }

        // TODO: Concempts
        template <typename T, typename = void> struct is_input_stream : std::false_type {};
        template <typename T> constexpr bool is_input_stream_v = is_input_stream<T>::value;

        template <typename T>
        struct is_input_stream<T, std::void_t<
            decltype(std::declval<T>().operator bool()),
            decltype(std::declval<T>().peek()),
            decltype(std::declval<T>().get()),
            decltype(std::declval<T>().eof())
            >> : std::true_type {};
    }

    template <typename InputStream, std::enable_if_t<details::is_input_stream_v<std::decay_t<InputStream>>, int> = 0>
    value parse(InputStream& input)
    {
        lexer lex(input);
        auto result = details::parse(lex);
        if (lex.current_token != lexer_token::eof)
        {
            throw std::runtime_error("Invalid JSON; too much data (" + lex.string_value + ")");
        }
        return result;
    }

    inline value parse(std::string_view text)
    {
        buffer_input_stream stream(text);
        return parse(stream);
    }

    template <typename InputStream, std::enable_if_t<details::is_input_stream_v<std::decay_t<InputStream>>, int> = 0>
    null_t parse_null(InputStream& input)
    {
        lexer lex(input);
        auto result = details::parse_null(lex);
        if (lex.current_token != lexer_token::eof)
        {
            throw std::runtime_error("Invalid JSON; too much data (" + lex.string_value + ")");
        }
        return result;
    }

    inline null_t parse_null(std::string_view text)
    {
        buffer_input_stream stream(text);
        return parse_null(stream);
    }

    template <typename InputStream, std::enable_if_t<details::is_input_stream_v<std::decay_t<InputStream>>, int> = 0>
    boolean_t parse_boolean(InputStream& input)
    {
        lexer lex(input);
        auto result = details::parse_boolean(lex);
        if (lex.current_token != lexer_token::eof)
        {
            throw std::runtime_error("Invalid JSON; too much data (" + lex.string_value + ")");
        }
        return result;
    }

    inline boolean_t parse_boolean(std::string_view text)
    {
        buffer_input_stream stream(text);
        return parse_boolean(stream);
    }

    template <typename InputStream, std::enable_if_t<details::is_input_stream_v<std::decay_t<InputStream>>, int> = 0>
    number_t parse_number(InputStream& input)
    {
        lexer lex(input);
        auto result = details::parse_number(lex);
        if (lex.current_token != lexer_token::eof)
        {
            throw std::runtime_error("Invalid JSON; too much data (" + lex.string_value + ")");
        }
        return result;
    }

    inline number_t parse_number(std::string_view text)
    {
        buffer_input_stream stream(text);
        return parse_number(stream);
    }

    template <typename InputStream, std::enable_if_t<details::is_input_stream_v<std::decay_t<InputStream>>, int> = 0>
    string_t parse_string(InputStream& input)
    {
        lexer lex(input);
        auto result = details::parse_string(lex);
        if (lex.current_token != lexer_token::eof)
        {
            throw std::runtime_error("Invalid JSON; too much data (" + lex.string_value + ")");
        }
        return result;
    }

    inline string_t parse_string(std::string_view text)
    {
        buffer_input_stream stream(text);
        return parse_string(stream);
    }

    template <typename InputStream, std::enable_if_t<details::is_input_stream_v<std::decay_t<InputStream>>, int> = 0>
    array_t<> parse_array(InputStream& input)
    {
        lexer lex(input);
        auto result = details::parse_array(lex);
        if (lex.current_token != lexer_token::eof)
        {
            throw std::runtime_error("Invalid JSON; too much data (" + lex.string_value + ")");
        }
        return result;
    }

    inline array_t<> parse_array(std::string_view text)
    {
        buffer_input_stream stream(text);
        return parse_array(stream);
    }

    template <typename InputStream, std::enable_if_t<details::is_input_stream_v<std::decay_t<InputStream>>, int> = 0>
    object_t parse_object(InputStream& input)
    {
        lexer lex(input);
        auto result = details::parse_object(lex);
        if (lex.current_token != lexer_token::eof)
        {
            throw std::runtime_error("Invalid JSON; too much data (" + lex.string_value + ")");
        }
        return result;
    }

    inline object_t parse_object(std::string_view text)
    {
        buffer_input_stream stream(text);
        return parse_object(stream);
    }
}

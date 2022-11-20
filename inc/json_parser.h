#pragma once

#include <charconv>
#include <system_error>

#include "json_lexer.h"

namespace json
{
    template <typename InputStream, typename PointerT>
    inline bool parse_null(lexer<InputStream>& lexer, PointerT& target) noexcept
    {
        if (lexer.current_token != lexer_token::keyword_null) return false;

        target = nullptr;
        lexer.advance();
        return true;
    }

    template <typename InputStream, typename BoolT>
    inline bool parse_true(lexer<InputStream>& lexer, BoolT& target) noexcept
    {
        if (lexer.current_token != lexer_token::keyword_true) return false;

        target = true;
        lexer.advance();
        return true;
    }

    template <typename InputStream, typename BoolT>
    inline bool parse_false(lexer<InputStream>& lexer, BoolT& target) noexcept
    {
        if (lexer.current_token != lexer_token::keyword_false) return false;

        target = false;
        lexer.advance();
        return true;
    }

    // Helper to consume either a 'true' or 'false' token
    template <typename InputStream, typename BoolT>
    inline bool parse_bool(lexer<InputStream>& lexer, BoolT& target) noexcept
    {
        return parse_true(lexer, target) || parse_false(lexer, target);
    }

    template <typename InputStream, typename StringT>
    inline bool parse_string(lexer<InputStream>& lexer, StringT& target)
    {
        if (lexer.current_token != lexer_token::string) return false;

        target = std::move(lexer.string_value);
        lexer.advance();
        return true;
    }

    template <typename InputStream, typename NumberT>
    inline bool parse_number(lexer<InputStream>& lexer, NumberT& target) noexcept
    {
        if (lexer.current_token != lexer_token::number) return false;

        auto fromChars = [](std::string_view str, auto& number) {
            auto begin = str.data();
            auto end = begin + str.size();
            auto [ptr, ec] = std::from_chars(begin, end, number);
            return (ptr == end) && (ec == std::errc{});
        };

        NumberT result;
        if constexpr (std::is_floating_point_v<NumberT>)
        {
            if (!fromChars(lexer.string_value, result)) return false;
        }
        else if constexpr (sizeof(NumberT) <= 6)
        {
            // <= 48-bit integer can fit inside of a 64-bit double, which is much simpler to parse and verify
            double numberFloat;
            if (!fromChars(lexer.string_value, numberFloat)) return false;

            result = static_cast<NumberT>(numberFloat);
            if (static_cast<double>(result) != numberFloat) return false;
        }
        else
        {
            // > 48-bit integer cannot fit inside of a 64-bit double, so we need to do a bit of manual work
            bool negative = false;
            std::string_view coeffWholeStr;
            std::string_view coeffFractionStr;
            std::string_view exponentStr;

            std::string_view remainder = lexer.string_value;
            if (remainder.front() == '-')
            {
                if constexpr (!std::is_signed_v<NumberT>) return false;

                negative = true;
                remainder = remainder.substr(1);
            }

            auto pos = remainder.find_first_of(".eE");
            if (pos == std::string::npos)
            {
                coeffWholeStr = remainder;
            }
            else
            {
                auto ch = remainder[pos];
                coeffWholeStr = remainder.substr(0, pos);
                remainder = remainder.substr(pos + 1);

                if (ch == '.')
                {
                    pos = remainder.find_first_of("eE");
                    if (pos == std::string::npos)
                    {
                        // We need to allow something like '42.0'
                        coeffFractionStr = remainder;
                    }
                    else
                    {
                        coeffFractionStr = remainder.substr(0, pos);
                        exponentStr = remainder.substr(pos + 1);
                    }
                }
                else
                {
                    exponentStr = remainder;
                }

                if (!exponentStr.empty() && (exponentStr.front() == '+'))
                {
                    exponentStr = exponentStr.substr(1); // from_chars doesn't allow a leading '+'
                }
            }

            int exponent = 0;
            if (!exponentStr.empty() && !fromChars(exponentStr, exponent)) return false;

            std::make_unsigned_t<NumberT> resultUnsigned;
            if (exponent <= 0)
            {
                // E.g. must be something like '420.0e-1' to remain integral
                if (coeffFractionStr.find_first_not_of('0') != std::string_view::npos) return false;

                auto shiftLen = std::min(static_cast<std::size_t>(-exponent), coeffWholeStr.size());
                auto coeffWholeUpper = coeffWholeStr.substr(0, coeffWholeStr.size() - shiftLen);
                auto coeffWholeLower = coeffWholeStr.substr(coeffWholeUpper.size());
                if (coeffWholeLower.find_first_not_of('0') != std::string_view::npos) return false;

                // The remainder is our number
                if (!fromChars(coeffWholeUpper, resultUnsigned)) return false;
            }
            else
            {
                // E.g. must be something like '4.20e1' if there's a fraction
                auto shiftLen = std::min(static_cast<std::size_t>(exponent), coeffFractionStr.size());
                auto coeffFractionUpper = coeffFractionStr.substr(0, shiftLen);
                auto coeffFractionLower = coeffFractionStr.substr(shiftLen);
                if (coeffFractionLower.find_first_not_of('0') != std::string_view::npos) return false;

                auto pow10 = [&](int amt) {
                    for (int i = 0; i < amt; ++i)
                    {
                        // TODO: Better way to do this and detect overflow?
                        auto newResult = resultUnsigned * 10;
                        if (newResult < resultUnsigned) return false;
                        resultUnsigned = newResult;
                    }
                    return true;
                };

                // We're going to read the fraction as-is, so multiply by the power of 10 up to the shift
                if (!fromChars(coeffWholeStr, resultUnsigned)) return false;
                if (!pow10(static_cast<int>(shiftLen))) return false;

                std::make_unsigned_t<NumberT> fraction;
                if (!fromChars(coeffFractionUpper, fraction)) return false;

                auto newResult = resultUnsigned + fraction;
                if (newResult < resultUnsigned) return false;
                resultUnsigned = newResult;

                if (!pow10(exponent - static_cast<int>(shiftLen))) return false;
            }

            if (negative)
            {
                result = static_cast<NumberT>(~resultUnsigned + 1);
                if (result > 0) return false;
            }
            else
            {
                result = static_cast<NumberT>(resultUnsigned);
                if constexpr (std::is_signed_v<NumberT>)
                {
                    if (result < 0) return false;
                }
            }
        }

        target = result;
        lexer.advance();
        return true;
    }

    template <typename InputStream, typename ObjectT, typename Callback>
    inline bool parse_object(lexer<InputStream>& lexer, ObjectT&& target, Callback&& callback)
    {
        if (lexer.current_token != lexer_token::curly_open) return false;
        lexer.advance();

        if (lexer.current_token != lexer_token::curly_close)
        {
            std::string name;
            while (true)
            {
                // NOTE: Specifically not calling 'parse_string' here so that we can continuously exchange buffers
                if (lexer.current_token != lexer_token::string) return false;
                name.swap(lexer.string_value);
                lexer.advance();

                if (lexer.current_token != lexer_token::colon) return false;
                lexer.advance();

                // Don't invoke the callback for invalid/eof
                if ((lexer.current_token == lexer_token::invalid) || (lexer.current_token == lexer_token::eof))
                    return false;

                if (!callback(lexer, target, name)) return false;
                // NOTE: 'callback' should consume all tokens it needs

                if (lexer.current_token != lexer_token::comma) break;
                lexer.advance();
            }
        }

        if (lexer.current_token != lexer_token::curly_close) return false;
        lexer.advance();

        return true;
    }

    template <typename InputStream, typename ArrayT, typename Callback>
    inline bool parse_array(lexer<InputStream>& lexer, ArrayT&& target, Callback&& callback)
    {
        if (lexer.current_token != lexer_token::bracket_open) return false;
        lexer.advance();

        // Don't invoke the callback for invalid/eof
        if ((lexer.current_token == lexer_token::invalid) || (lexer.current_token == lexer_token::eof)) return false;

        if (lexer.current_token != lexer_token::bracket_close)
        {
            while (true)
            {
                if (!callback(lexer, target)) return false;
                // NOTE: 'callback' should consume all tokens it needs

                if (lexer.current_token != lexer_token::comma) break;
                lexer.advance();
            }
        }

        if (lexer.current_token != lexer_token::bracket_close) return false;
        lexer.advance();

        return true;
    }

    namespace details
    {
        template <typename InputStream>
        inline bool ignore_single_token(lexer<InputStream>& lexer, lexer_token token) noexcept
        {
            if (lexer.current_token != token) return false;

            lexer.advance();
            return true;
        }
    }

    template <typename InputStream>
    inline bool ignore_null(lexer<InputStream>& lexer) noexcept
    {
        return details::ignore_single_token(lexer, lexer_token::keyword_null);
    }

    template <typename InputStream>
    inline bool ignore_true(lexer<InputStream>& lexer) noexcept
    {
        return details::ignore_single_token(lexer, lexer_token::keyword_true);
    }

    template <typename InputStream>
    inline bool ignore_false(lexer<InputStream>& lexer) noexcept
    {
        return details::ignore_single_token(lexer, lexer_token::keyword_false);
    }

    template <typename InputStream>
    inline bool ignore_bool(lexer<InputStream>& lexer) noexcept
    {
        return ignore_true(lexer) || ignore_false(lexer);
    }

    template <typename InputStream>
    inline bool ignore_string(lexer<InputStream>& lexer) noexcept
    {
        return details::ignore_single_token(lexer, lexer_token::string);
    }

    template <typename InputStream>
    inline bool ignore_number(lexer<InputStream>& lexer) noexcept
    {
        return details::ignore_single_token(lexer, lexer_token::number);
    }

    template <typename InputStream>
    inline bool ignore_array(lexer<InputStream>& lexer) noexcept;

    template <typename InputStream>
    inline bool ignore_object(lexer<InputStream>& lexer) noexcept
    {
        return parse_object(lexer, nullptr, [](auto&& lexer, auto&&, auto&&) {
            switch (lexer.current_token)
            {
            case lexer_token::curly_open: return ignore_object(lexer);
            case lexer_token::bracket_open: return ignore_array(lexer);
            case lexer_token::keyword_true: return ignore_true(lexer);
            case lexer_token::keyword_false: return ignore_false(lexer);
            case lexer_token::keyword_null: return ignore_null(lexer);
            case lexer_token::string: return ignore_string(lexer);
            case lexer_token::number: return ignore_number(lexer);
            default: return false;
            }
        });
    }

    template <typename InputStream>
    inline bool ignore_array(lexer<InputStream>& lexer) noexcept
    {
        return parse_array(lexer, nullptr, [](auto&& lexer, auto&&) {
            switch (lexer.current_token)
            {
            case lexer_token::curly_open: return ignore_object(lexer);
            case lexer_token::bracket_open: return ignore_array(lexer);
            case lexer_token::keyword_true: return ignore_true(lexer);
            case lexer_token::keyword_false: return ignore_false(lexer);
            case lexer_token::keyword_null: return ignore_null(lexer);
            case lexer_token::string: return ignore_string(lexer);
            case lexer_token::number: return ignore_number(lexer);
            default: return false;
            }
        });
    }
}

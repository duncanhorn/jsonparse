#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "json_parser.h"

namespace json
{
    struct value;

    namespace details
    {
        struct string_hash
        {
            using is_transparent = void;

            std::size_t operator()(const std::string_view& str) const noexcept
            {
                return std::hash<std::string_view>{}(str);
            }
        };
    }

    using null = std::nullptr_t;
    using boolean = bool;
    using number = double;
    using string = std::string;
    using array = std::vector<value>;
    using object = std::unordered_map<string, value, details::string_hash, std::equal_to<>>;

    struct value
    {
        using type = std::variant<std::monostate, null, boolean, number, string, array, object>;
        type data;

        value() = default;

        template <typename T, std::enable_if_t<std::is_constructible_v<type, T&&>, int> = 0>
        value(T&& value) : data(std::forward<T>(value))
        {
        }

        template <typename T>
        constexpr T* get() noexcept
        {
            return std::get_if<T>(&data);
        }

        template <typename T>
        constexpr const T* get() const noexcept
        {
            return std::get_if<T>(&data);
        }

        constexpr null* get_null() noexcept
        {
            return get<null>();
        }

        constexpr const null* get_null() const noexcept
        {
            return get<null>();
        }

        constexpr boolean* get_boolean() noexcept
        {
            return get<boolean>();
        }

        constexpr const boolean* get_boolean() const noexcept
        {
            return get<boolean>();
        }

        constexpr number* get_number() noexcept
        {
            return get<number>();
        }

        constexpr const number* get_number() const noexcept
        {
            return get<number>();
        }

        constexpr string* get_string() noexcept
        {
            return get<string>();
        }

        constexpr const string* get_string() const noexcept
        {
            return get<string>();
        }

        constexpr array* get_array() noexcept
        {
            return get<array>();
        }

        constexpr const array* get_array() const noexcept
        {
            return get<array>();
        }

        constexpr object* get_object() noexcept
        {
            return get<object>();
        }

        constexpr const object* get_object() const noexcept
        {
            return get<object>();
        }
    };

    template <InputStream InputStreamT>
    inline bool parse_value(lexer<InputStreamT>& lexer, value& target) noexcept
    {
        switch (lexer.current_token)
        {
        case lexer_token::curly_open: {
            auto& obj = target.data.emplace<object>();
            return parse_object(lexer, obj, [](auto& lexer, auto& obj, auto& name) {
                value v;
                if (!parse_value(lexer, v)) return false;
                auto pair = obj.emplace(name, std::move(v));
                return pair.second;
            });
        }

        case lexer_token::bracket_open: {
            auto& arr = target.data.emplace<array>();
            return parse_array(lexer, arr, [](auto& lexer, auto& arr) {
                value v;
                if (!parse_value(lexer, v)) return false;
                arr.push_back(std::move(v));
                return true;
            });
        }

        case lexer_token::keyword_true:
            target.data.emplace<boolean>(true);
            lexer.advance();
            return true;

        case lexer_token::keyword_false:
            target.data.emplace<boolean>(false);
            lexer.advance();
            return true;

        case lexer_token::keyword_null:
            target.data.emplace<null>(nullptr);
            lexer.advance();
            return true;

        case lexer_token::string:
            target.data.emplace<string>(lexer.string_value);
            lexer.advance();
            return true;

        case lexer_token::number: {
            auto& num = target.data.emplace<number>(0);
            return parse_number(lexer, num);
        }

        default: return false;
        }
    }

    inline value* object_get(object& obj, std::string_view name) noexcept
    {
        auto itr = obj.find(name);
        if (itr == obj.end())
        {
            return nullptr;
        }

        return &itr->second;
    }

    inline const value* object_get(const object& obj, std::string_view name) noexcept
    {
        auto itr = obj.find(name);
        if (itr == obj.end())
        {
            return nullptr;
        }

        return &itr->second;
    }

    template <typename T>
    inline T* object_get_as(object& obj, std::string_view name) noexcept
    {
        auto value = object_get(obj, name);
        if (!value) return nullptr;

        return value->get<T>();
    }

    template <typename T>
    inline const T* object_get_as(const object& obj, std::string_view name) noexcept
    {
        auto value = object_get(obj, name);
        if (!value) return nullptr;

        return value->get<T>();
    }
}

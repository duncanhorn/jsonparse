# jsonparse
A simple C++ library for parsing JSON text.
The philosophy of the library is to provide a set of building blocks to give the consumer maximum control over the operation of the library.
At its lowest level, the [`json::lexer`](inc/json_lexer.h) type is used to convert an input stream into a series of tokens.
This avoids any unnecessary "intermediate representation" if the data is going to be used to initialize a native object.
E.g. a "typical" JSON library will likely do something like parse objects as maps of `(key, value)` pairs.
This is not only inefficient if that's not the desired final representation, but it's also just as tedious to perform invalid/missing value checks as it is to use the `json::lexer` type directly.
The library also provides a set of "value types" and a single `json::parse` function for those who want the more "standard" consumption that gives the familiar objects-as-maps experience.

# The `json::lexer` Type
At its heart, this library uses the `json::lexer` type to convert characters read from an input stream into `json::lexer_token`s.
The application will then use these tokens to initialize its internal data structures.
For example, lets suppose that we have a structure `foo` defined as follows:
```c++
struct foo
{
    bool flag = false;
    std::string text;
    int number;
};
```
If we wanted to parse JSON text as a `foo` object, we could write:
```c++
using json_lexer = json::lexer<json::buffer_input_stream>;

bool parse_foo(json_lexer& lexer, foo* target)
{
    if (lexer.current_token != json::lexer_token::curly_open) return false;
    lexer.advance();

    if (lexer.current_token != json::lexer_token::curly_close)
    {
        std::string name;
        while (true)
        {
            if (lexer.current_token != json::lexer_token::string) return false;
            std::swap(name, lexer.string_value);
            lexer.advance();

            if (lexer.current_token != json::lexer_token::colon) return false;
            lexer.advance();

            if (name == "flag")
            {
                if (!parse_bool(lexer, &target->flag)) return false;
            }
            else if (name == "text")
            {
                if (!parse_string(lexer, &target->text)) return false;
            }
            else if (name == "number")
            {
                if (!parse_int(lexer, &target->text)) return false;
            }
            else
            {
                return false;
            }

            if (lexer.current_token != json::lexer_token::comma) break;
            lexer.advance();
        }
    }

    if (lexer.current_token != json::lexer_token::curly_close) return false;
    lexer.advance();

    return true;
}
```

#

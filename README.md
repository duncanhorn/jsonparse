# jsonparse
A simple C++ library for parsing JSON text.
The philosophy of the library is to provide a set of building blocks to give the consumer maximum control over the operation of the library.
At its lowest level, the [`json::lexer`](inc/json_lexer.h) type is used to convert an input stream into a series of tokens.
This avoids any unnecessary "intermediate representation" if the data is going to be used to initialize a native object.
E.g. a "typical" JSON library will likely do something like parse objects as maps of `(key, value)` pairs and represent values using some `variant`/`union` type.
This is not only inefficient if that's not the desired final representation, but it's also often just as tedious to convert to the final representation.

One "step up" from the `json::lexer` type are the [`json::parse_*`](inc/json_parser.h) functions.
These functions are mostly just helpers around the `json::lexer` type, taking care of some of the more tedious steps such as type checking, handling commas/colons/braces, and advancing the lexer's current token.
These functions still impose few requirements on the destination types.
For `parse_null`, `parse_true`/`parse_false`/`parse_bool`, and `parse_string`, assignment from `nullptr`, `true`/`false`, and `std::string` will be used as appropriate.
For `parse_number`, `std::from_chars` is used, along with some custom logic to ensure that something like `4.2e1` can be parsed to a non-floating point type such as an `int`.
Both `parse_array` and `parse_object` use callbacks to communicate the next value in the collection.
This data is again presented "as-is", i.e. there is no missing/duplicated value checks.

The library also provides a [`json::value`](inc/json.h) type as well as a single `json::parse_value` function for those who want the more "familiar" consumption that gives the values as variants experience.
The `json::value` type contains a single `data` member of type `std::variant` that holds the underlying value, along with a number of helper functions for accessing by type (e.g. `get_number`, `get_array`, etc.).
These functions all return pointers, using `nullptr` to communicate that the requested type is not held by the value.
Note that `get_null` returns a pointer to a `std::nullptr_t`.
That is, a `null` return value means failure, not success.

## The `json::lexer` Type
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
                if (lexer.current_token == json::lexer_token::keyword_true)
                {
                    target->flag = true;
                    lexer.advance();
                }
                else if (lexer.current_token == json::lexer_token::keyword_false)
                {
                    target->flag = false;
                    lexer.advance();
                }
                else return false;
            }
            else if (name == "text")
            {
                if (lexer.current_token == json::lexer_token::string)
                {
                    target->text = lexer.string_value;
                    lexer.advance();
                }
                else return false;
            }
            else if (name == "number")
            {
                if (lexer.current_token == json::lexer_token::number)
                {
                    target->number = std::atoi(lexer.string_value.c_str()); // Error handling not shown
                    lexer.advance();
                }
                else return false;
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

As you can see, this is fairly tedious and error-prone.
It's doing a bunch of tasks that aren't specific to this this scenario, such as validating the structure of the object and checking and advancing the lexer's current token.
That's where the various parsing functions come in.

## The `json::parse_*` Functions
The library provides a `parse_*` function for all of the different JSON value types, as well as individual ones for `true` and `false` as they are each different tokens.
With the exception of object and array types, the parsing functions assign directly to the target value.
Both object and array types use callbacks to communicate embedded values, with the object callback accepting an additional string argument for the member's name.
In all cases, the return type of each parsing function is `bool` to communicate success/failure.
The callbacks (if applicable) are also expected to return a `bool` value to communicate success/failure back to the library function.
With these helpers, the code from above becomes much simpler to write:

```c++
using json_lexer = json::lexer<json::buffer_input_stream>;

bool parse_foo(json_lexer& lexer, foo* target)
{
    return json::parse_object(lexer, *target, [](auto& lexer, auto& name, auto& target) {
        if (name == "flag")
        {
            return json::parse_bool(lexer, target.flag);
        }
        else if (name == "text")
        {
            return json::parse_string(lexer, target.text);
        }
        else if (name == "number")
        {
            return json::parse_number(lexer, target.number);
        }

        return false;
    });
}
```

## The `json::value` Type
If a native representation does not exist or is too complicated to represent, the `json::value` type exists to represent an arbitrary JSON value type.
This type uses `std::variant` under the hood - accessible via the `data` member - to hold the underlying data.
The following table describes the types that this variant can hold, with `std::monostate` representing a `json::value` that has not been initialized.

|JSON Value Type|Alias|Underlying Type|
|-|-|-|
|N/A|N/A|`std::monostate`|
|Null|`json::null`|`std::nullptr_t`|
|Boolean|`json::boolean`|`bool`|
|Number|`json::number`|`double`|
|String|`json::string`|`std::string`|
|Array|`json::array`|`std::vector<json::value>`|
|Object|`json::object`|`std::unordered_map<json::string, json::value>`|

The library also provides the `json::parse_value` function to parse tokens from a `json::lexer` into a `json::value`.
As an example, we could rewrite the code from above as follows:

```c++
using json_lexer = json::lexer<json::buffer_input_stream>;

bool parse_foo(json_lexer& lexer, foo* target)
{
    json::value value;
    if (!json::parse_value(lexer, value)) return false;

    auto obj = value.get_object();
    if (!obj) return false;

    if (auto flag = json::object_get_as<json::boolean>(*obj, "flag")) target->flag = *flag;
    else return false;

    if (auto text = json::object_get_as<json::string>(*obj, "text")) target->text = *text;
    else return false;

    if (auto number = json::object_get_as<json::number>(*obj, "number")) target->number = static_cast<int>(*number);
    else return false;

    return true;
}
```

Now, this is just an example of how to consume the API, not an endorsement of using the type in this way.
In fact, there's more code here than in the previous example!
If you plan to parse data in to native objects, it's arguably best to do so using the other parsing functions.
This example _does_ have the benefit that it verifies that all members are present, however it does not validate against "extra" members (although a size check could easily be added).

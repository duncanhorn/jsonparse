
#include <json_lexer.h>

int unicode_tests();
int lexer_tests();
int parser_tests();
int value_tests();

int main()
{
    int result = 0;
    result += unicode_tests();
    result += lexer_tests();
    result += parser_tests();
    result += value_tests();
    return result;
}

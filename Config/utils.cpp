#include "Tokenizer.hpp"


bool is_special_char(char c)
{
    return (c == '\n' || c == ';' || c == '{' || c == '}' || c == '"' || c == '\'' || c == '#');
}

bool is_whitespace(char c)
{
    return (c == 32 || c == 9 || (c >= 11 && c <= 13));
}
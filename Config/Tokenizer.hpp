#pragma once
#include "../main.hpp"

bool is_special_char(char c);
bool is_whitespace(char c);

enum State
{
    STRING,
    QUOTE,
    SYMBOL,
    WHITESPACE,
    COMMENT,
    NEWLINE,
    UNKNOWN
};
typedef struct s_state
{
    State state;
    std::string value;
} t_state;

enum TokenType
{
    VALUE,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    SEMICOLON,
    KEY,
    BLOCK,
    AFTER_BLOCK,
    LOCATION_KEY,
    UNDEFINED,
};
typedef struct s_token
{
    TokenType type;
    std::string value;
} t_token;


class Tokenizer
{
private:
    std::vector<std::string> lines;
public:
    Tokenizer();
    ~Tokenizer();
    void processFile(std::ifstream &file);
    std::vector<std::string> getLines();
    void processLines(const std::string &lines, std::vector<t_state> &F_states);
    bool processStates(std::vector<t_state> &F_states);
    bool tokenizeStates(std::vector<t_state> &F_states, std::vector<t_token> &tokens);
    std::vector<t_token> tokenize(std::string lines);

};

#include "Tokenizer.hpp"

Tokenizer::Tokenizer() {}
Tokenizer::~Tokenizer() {}

void Tokenizer::processFile(std::ifstream &file)
{
    if (!file.is_open())
    {
        print_message("Error: Could not open config file", RED);
        exit(1);
    }
    std::string server_block, line;
    while (std::getline(file, line))
    {
        if (line == "# ==================== End Server Block ====================")
        {
            this->lines.push_back(server_block);
            server_block.clear();
        }
        else
            server_block += line + '\n';
    }
    if (!server_block.empty())
        this->lines.push_back(server_block);
    if (this->lines.empty())
    {
        print_message("Error: empty file or no server blocks", RED);
        file.close();
        exit(1);
    }
    size_t i = 0;
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
    {
        i = 0;
        for (std::string::iterator it2 = it->begin(); it2 != it->end(); ++it2)
        {
            if (is_whitespace(*it2) || *it2 == '\n')
                i++;
        }
        if (i == it->size())
        {
            print_message("Error: empty server block", RED);
            file.close();
            exit(1);
        }
    }
}

std::vector<std::string> Tokenizer::getLines() { return this->lines; }

void Tokenizer::processLines(const std::string &lines, std::vector<t_state> &F_states)
{
    std::string::const_iterator it = lines.begin();
    while (it != lines.end())
    {
        t_state state = {UNKNOWN, ""};
        if (is_special_char(*it))
        {
            state.value = *it;
            if (*it == '\n')
                state.state = NEWLINE;
            else if (*it == '#')
                state.state = COMMENT;
            else if (*it == ';' || *it == '{' || *it == '}')
                state.state = SYMBOL;
            else if (*it == '"' || *it == '\'')
                state.state = QUOTE;
            ++it;
        }
        else if (is_whitespace(*it))
        {
            std::string::const_iterator start = it;
            while (it != lines.end() && is_whitespace(*it))
                ++it;
            state.value = std::string(start, it);
            state.state = WHITESPACE;
        }
        else
        {
            std::string::const_iterator start = it;
            while (it != lines.end() && !is_special_char(*it) && !is_whitespace(*it))
                ++it;
            state.value = std::string(start, it);
            state.state = STRING;
        }
        F_states.push_back(state);
    }
}

bool Tokenizer::processStates(std::vector<t_state> &F_states)
{
    char active_quote = '\0';
    std::vector<t_state>::iterator it = F_states.begin();
    while (it != F_states.end())
    {
        int j = 0;
        if (it->state == QUOTE)
        {
            if (active_quote == '\0')
            {
                active_quote = it->value[0];
                it = F_states.erase(it);
            }
            else if (active_quote == it->value[0])
            {
                active_quote = '\0';
                it = F_states.erase(it);
            }
            else
                j = 1;
        }
        else if (active_quote != '\0')
        {
            std::vector<t_state>::iterator start = it;
            std::string value;
            while (it != F_states.end() && !(it->state == QUOTE && it->value[0] == active_quote))
            {
                if (it->state == NEWLINE)
                    it = F_states.erase(it);
                else
                {
                    value += it->value;
                    it = F_states.erase(it);
                }
            }
            if (it != F_states.end() && it->state == QUOTE && it->value[0] == active_quote)
            {
                active_quote = '\0';
                it = F_states.erase(it);
            }
            t_state state = {STRING, value};
            it = F_states.erase(start, it);
            if (value == "{" || value == "}" || value == ";")
                state.state = SYMBOL;
            else
            {
                std::string::const_iterator it = value.begin();
                while (it != value.end() && (is_whitespace(*it) || *it == '\n'))
                    ++it;
                if (it == value.end())
                    state.state = WHITESPACE;
            }
            it = F_states.insert(it, state);
            ++it;
        }
        else
        {
            if (it->state == WHITESPACE)
                it = F_states.erase(it);
            else if (it->state == COMMENT)
            {
                while (it != F_states.end() && it->state != NEWLINE)
                    it = F_states.erase(it);
            }
            else
                ++it;
        }
        if (j == 1)
            it->state = STRING;
    }
    for (std::vector<t_state>::iterator it = F_states.begin(); it != F_states.end(); ++it)
    {
        if (it->state == WHITESPACE)
        {
            F_states.erase(it);
            --it;
        }
        if (it->state == NEWLINE && (it + 1) != F_states.end() && (it + 1)->state == NEWLINE)
        {
            F_states.erase(it);
            --it;
        }
    }
    for (std::vector<t_state>::iterator it = F_states.begin(); it != F_states.end(); ++it)
    {
        if (it->state == SYMBOL && ((it + 1) != F_states.end() && (it + 1)->state != NEWLINE))
        {
            t_state state = {NEWLINE, "\n"};
            it = F_states.insert(it + 1, state);
        }
    }
    for (std::vector<t_state>::iterator it = F_states.begin(); it != F_states.end(); ++it)
    {
        if (it->state == SYMBOL && (it->value == "{" || it->value == ";"))
        {
            if (it != F_states.begin() && (it - 1)->state == NEWLINE && (it + 1) != F_states.end() && (it + 1)->state == NEWLINE)
            {
                F_states.erase(it - 1);
                --it;
            }
        }
    }
    for (std::vector<t_state>::iterator it = F_states.begin(); it != F_states.end(); ++it)
    {
        if (it->state == SYMBOL && it->value == ";")
        {
            if (it != F_states.begin() && ((it - 1)->state == NEWLINE || (it - 1)->state == SYMBOL))
                return (print_message("Error: unexpected ; after newline or another symbol", RED), false);
            if ((it + 1) != F_states.end() && (it + 1)->value == "{")
                return (print_message("Error: unexpected ; before {", RED), false);
        }
    }
    if (active_quote != '\0')
        return (print_message("Error: unclosed quote", RED), false);
    return true;
}

bool Tokenizer::tokenizeStates(std::vector<t_state> &F_states, std::vector<t_token> &tokens)
{
    int i = 0, j = 0;
    for (std::vector<t_state>::iterator it = F_states.begin(); it != F_states.end(); ++it)
    {
        t_token token = {UNDEFINED, ""};
        if (it->state == SYMBOL)
        {
            if (it->value == "{")
                token.type = OPEN_BRACKET;
            else if (it->value == "}")
                token.type = CLOSE_BRACKET;
            else if (it->value == ";")
                token.type = SEMICOLON;
            token.value = it->value;
        }
        else if (it->state == STRING)
        {
            std::vector<t_state> tmp;
            for (std::vector<t_state>::iterator it2 = it; it2 != F_states.end(); ++it2)
            {
                if (it2->state == NEWLINE)
                    break;
                if (it2->state == SYMBOL && it2->value == "{")
                    i = 1;
            }
            if (i == 1 && j == 0)
            {
                token.type = BLOCK;
                j = 1;
            }
            else if (it != F_states.begin() && (tokens.back().type == BLOCK || tokens.back().type == AFTER_BLOCK))
                token.type = AFTER_BLOCK;
            else if (it == F_states.begin() || (it - 1)->state == NEWLINE)
                token.type = KEY;
            else
                token.type = VALUE;
            token.value = it->value;
            i = 0;
        }
        else if (it->state == NEWLINE)
        {
            j = 0;
            continue;
        }
        tokens.push_back(token);
    }
    int open_brackets = 0, close_brackets = 0;
    for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (it->type == VALUE && (it + 1) != tokens.end())
        {
            if ((it + 1)->type == VALUE)
            {
                while ((it + 1) != tokens.end() && (it + 1)->type == VALUE)
                    ++it;
                if ((it + 1) == tokens.end() || (it + 1)->type != SEMICOLON)
                    return (print_message("Error: missing ; after value", RED), false);
            }
            else if ((it + 1)->type != SEMICOLON)
                return (print_message("Error: missing ; after value", RED), false);
        }
        if (it->type == OPEN_BRACKET)
            open_brackets++;
        else if (it->type == CLOSE_BRACKET)
            close_brackets++;
    }
    if (open_brackets != close_brackets)
        return (print_message("Error: You have an unclosed block", RED), false);
    for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (it->type == SEMICOLON && (it == tokens.begin() || (it - 1)->type != VALUE))
            return (print_message("Error: unexpected ; after something other than value", RED), false);
        else if (it->type == CLOSE_BRACKET && (it == tokens.begin() || ((it - 1)->type != SEMICOLON && (it - 1)->type != CLOSE_BRACKET && (it - 1)->type != OPEN_BRACKET)))
            return (print_message("Error: unexpected } after something other than ; or {", RED), false);
        else if (it->type == OPEN_BRACKET && (it == tokens.begin() || ((it - 1)->type != BLOCK && (it - 1)->type != AFTER_BLOCK)))
            return (print_message("Error: unexpected { after something other than block or after block", RED), false);
    }
    for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (it->type == BLOCK && it->value == "location")
        {
            for (std::vector<t_token>::iterator it2 = it + 1; it2 != tokens.end() && it2->type != CLOSE_BRACKET; ++it2)
            {
                if (it2->type == KEY)
                    it2->type = LOCATION_KEY;
                else if (it2->type == BLOCK && it2->value == "location")
                    return (print_message("Error: You should not have a location block inside another location block", RED), false);
            }
        }
        if (it->type == KEY || it->type == LOCATION_KEY)
        {
            if ((it + 1) != tokens.end() && (it + 1)->type != VALUE)
                return (print_message("Error: You should have a value after key", RED), false);
        }
    }
    for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        if (it->type == SEMICOLON || it->type == OPEN_BRACKET || it->type == CLOSE_BRACKET)
        {
            tokens.erase(it);
            --it;
        }
        if (it->type == AFTER_BLOCK)
        {
            if ((it - 1)->type == AFTER_BLOCK || ((it - 1)->type == BLOCK && (it - 1)->value != "location"))
                return (print_message("Error: You should not have two after block tokens in a row", RED), false);
        }
    }
    return true;
}

std::vector<t_token> Tokenizer::tokenize(std::string lines)
{
    std::vector<t_state> F_states;
    std::vector<t_token> tokens;
    processLines(lines, F_states);
    if (!processStates(F_states))
        return tokens;
    if (!tokenizeStates(F_states, tokens))
        return std::vector<t_token>();
    return tokens;
}
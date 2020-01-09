#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <stdio.h>

#include <vector>

#include <token.hpp>
#include <token_stack.hpp>

class Tokenizer
{
private:
    char m_input[2];

    FILE* m_file;
    std::vector<char> m_buffer;
    TokenStack* m_stack;

private:
    Tokenizer();

    bool expect(char c);
    int read_escape_character();

    bool read_literal();
    bool read_identifier();
    bool read_punctuator();
    bool read_string();
    bool read_character();
    bool read_decimal();
    bool read_hexadecimal();
    bool read_single_line_comment();
    bool read_multi_line_comment();

    char pop();
    char peek(unsigned int offset);
    bool tokenize(FILE* m_file, TokenStack* stack);

public:
    static bool Tokenize(const char* file_path, TokenStack* stack);
};

#endif // TOKENIZER_HPP

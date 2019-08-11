#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <vector>

#include <token.hpp>
#include <token_stack.hpp>

class Tokenizer
{
private:
    const char*  m_buffer;
    unsigned int m_buffer_size;

    unsigned int m_read_ptr;

    std::vector<char> m_string_buffer;

    TokenStack m_tokens;

private:
    char pop();
    char peek(unsigned int offset);
    
    const char* current_ptr();

    bool read_single_line_comment();
    bool read_multi_line_comment();

    bool read_hex_value(Token& tk);
    bool read_dec_value(Token& tk);
    
    bool read_value(Token& tk);
    bool read_string(Token& tk);
    bool read_literal(Token& tk);
    bool read_character(Token& tk);
    
    bool read_escape_character(char& c);

    bool read_token(Token& tk);

    bool process(char c, Token& tk);
    bool process(const char* str, unsigned int len, Token& tk);

    bool next_token(Token& tk);

public:
    Tokenizer(const char* buffer, long size);
    
    bool tokenize();
    TokenStack& get_tokens();
};

#endif // TOKENIZER_HPP

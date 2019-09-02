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
    const char*  m_source;
    unsigned int m_source_size;

    unsigned int m_read_ptr;

    std::vector<char> m_buffer;

    TokenStack* m_stack;

private:
    char pop();
    char peek(unsigned int offset);
	const char* current_position();

    bool read_single_line_comment();
    bool read_multi_line_comment();

    bool read_hex_value(Token& tk);
    bool read_dec_value(Token& tk);
    
    bool read_value(Token& tk);
    bool read_string(Token& tk);
    
    bool read_escape_character(char& c);
    bool read_word(const char*& str_ptr, unsigned int& str_len);

	bool read_token();
	bool read_literal();
	bool read_identifier();
	bool read_character();
    
	bool skip_spaces();

    bool read_preprocessor();
    bool read_pp_include();

	bool process(char c);
	bool process(const char* str, unsigned int len);

	bool process(const char* source, unsigned int source_size, TokenStack* stack);

	Tokenizer();

	static bool Process(const char* path, TokenStack& stack);

public:
	static bool Tokenize(const char* path, TokenStack& stack);
};

#endif // TOKENIZER_HPP

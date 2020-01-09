#include <tokenizer.hpp>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <ascii.hpp>

Tokenizer::Tokenizer()
{
	m_file = nullptr;
	m_stack = nullptr;
}

int Tokenizer::read_escape_character()
{
	int value = 0;
	bool status = true;

	if(pop() != '\'')
	{
		status = false;
		printf("error: expected '\'\n");
	}

	if(status)
	{
		char c = pop();
		switch(c)
		{
			case '0':  { value = ASCII_NULL; break; }
			case 'n':  { value = ASCII_NEWLINE; break; }
			case 'r':  { value = ASCII_CARRIAGE_RETURN; break; }
			case 't':  { value = ASCII_TAB; break; }
			case '"':  { value = ASCII_DOUBLE_QUOTE; break; }
			case '\'': { value = ASCII_SINGLE_QUOTE; break; }
			case '\\': { value = ASCII_BACK_SLASH; break; }
			default:
			{
				if(c == 'x')
				{
					char str[3] = { 0 };
					for(unsigned int i = 0; status && (i < 2); i++)
					{
						c = pop();
						if(IS_HEXADECIMAL(c)) {
							str[i] = c;
						} else {
							status = false;
							printf("error: expected hexadecimal character\n");
						}
					}

					if(status) {
						value = (int) strtoul(str, nullptr, 16);
					}
				}
				break;
			}
		}
	}

	return status ? value : -1;
}

bool Tokenizer::expect(char c)
{
	bool status = true;

	if(pop() != c) {
		status = false;
		printf("error: expected \"%c\"\n", c);
	}

	return status;
}

bool Tokenizer::read_character()
{
	bool status = expect('\'');

	char c = 0;
	if(status)
	{
		c = pop();
		if(c == '\\')
		{
			int value = read_escape_character();
			status = (value != -1);

			if(status) {
				c = (char) value;
			}
		}
	}

	if(status)
	{
		m_stack->push({ TK_LITERAL, { LITERAL_CHAR, c }});
		status = expect('\'');
	}

	return status;
}

bool Tokenizer::read_string()
{
	bool status = expect('"');

	char c = 0;
	while(status)
	{
		c = pop();
		if(c == '"')
		{
			break;
		}
		
		if(c == '\\')
		{
			int value = read_escape_character();
			status = (value != -1);

			if(status) {
				c = (char) value;
			}
		}

		if(status) {
			m_buffer.push_back(c);
		}
	}

	if(status)
	{
		// TODO: STORE THE STRING
		m_buffer.clear();
	}

	return status;
}

bool Tokenizer::read_decimal()
{
	bool status = true;
	bool is_float = false;

	char c = 0;
	while(status)
	{
		c = peek(0);
		if(IS_NUM(c))
		{
			m_buffer.push_back(c);
		}
		else if(c == '.')
		{
			if(!is_float) {
				is_float = true;
			} else {
				status = false;
				printf("error: repeated decimal symbol\n");
			}
		}
		else
		{
			break;
		}
	}

	if(status)
	{
		m_buffer.push_back(0);
		m_stack->push({ TK_LITERAL, { LITERAL_INTEGER, strtoul(m_buffer.data(), nullptr, 10) } });
		m_buffer.clear();
	}

	return status;
}

bool Tokenizer::read_hexadecimal()
{
	bool status = expect('0') ? expect('x') : false;

	char c = 0;
	while(status)
	{
		c = peek(0);
		if(IS_HEXADECIMAL(c))
		{
			m_buffer.push_back(c);
		}
		else
		{
			break;
		}
	}

	if(status)
	{
		m_buffer.push_back(0);
		m_stack->push({ TK_LITERAL, { LITERAL_INTEGER, strtoul(m_buffer.data(), nullptr, 16) } });
		m_buffer.clear();
	}

	return status;
}

bool Tokenizer::read_literal()
{
	bool status = true;

	Token tk;
	char c = peek(0);
	if(c == '\'')
	{
		status = read_character();
	}
	else if(c == '"')
	{
		status = read_string();
	}
	else if((c == '0') && (peek(1) == 'x'))
	{
		status = read_hexadecimal();
	}
	else
	{
		status = read_decimal();
	}
	

	return status;
}

bool Tokenizer::read_single_line_comment()
{
	bool status = true;

	char c = 0;
	while(true)
	{
		c = pop();
		if(c == '\n')
		{
			break;
		}
	}

	return status;
}

bool Tokenizer::read_multi_line_comment()
{
	bool status = true;

	char c = 0;
	while(true)
	{
		c = pop();
		if(c == '*')
		{
			if(pop() == '/')
			{
				break;
			}
		}
		else if(c == EOF)
		{
			status = false;
			printf("error: could not find end of multi line comment\n");
		}
	}

	return status;
}

bool Tokenizer::read_identifier()
{
	bool status = true;

	char c = pop();
	if(IS_ALPHA(c))
	{
		m_buffer.push_back(c);
	}
	else
	{
		status = false;
		printf("error: expected alphabet character\n");
	}

	while(status)
	{
		c = pop();
		if(IS_ALPHA_NUM(c))
		{
			m_buffer.push_back(c);
		}
		else
		{
			break;
		}
	}

	if(status)
	{
		// TODO: Use the populated string here
	}

	return true;
}

bool Tokenizer::read_punctuator()
{
	bool status =  true;

	Token tk;
	char c = pop();
	
	switch(c)
	{
		case '<': { tk = { TK_LEFT_ARROW_HEAD };  break; }
		case '>': { tk = { TK_RIGHT_ARROW_HEAD }; break; }
		case '+': { tk = { TK_PLUS }; break; }
		case '-': { tk = { TK_MINUS }; break; }
		case '.': { tk = { TK_DOT }; break; }
		case '*': { tk = { TK_ASTERISK }; break; }
		case '/': { tk = { TK_FORWARD_SLASH }; break; }
		case '{': { tk = { TK_OPEN_CURLY_BRACKET }; break; }
		case '}': { tk = { TK_CLOSE_CURLY_BRACKET }; break; }
		case '(': { tk = { TK_OPEN_ROUND_BRACKET }; break; }
		case ')': { tk = { TK_CLOSE_ROUND_BRACKET }; break; }
		case '[': { tk = { TK_OPEN_SQUARE_BRACKET }; break; }
		case ']': { tk = { TK_CLOSE_SQUARE_BRACKET }; break; }
		case ';': { tk = { TK_SEMICOLON }; break; }
		case ',': { tk = { TK_COMMA }; break; }
		case '^': { tk = { TK_CARET }; break; }
		case '!': { tk = { TK_EXPLANATION_MARK }; break; }
		case '&': { tk = { TK_AMPERSAND }; break; }
		case '|': { tk = { TK_VERTICAL_BAR }; break; }
		case '%': { tk = { TK_PERCENT }; break; }
		case ':': { tk = { TK_COLON }; break; }
		default:
		{
			status = false;
			printf("unknown token '%c'\n", c);
			break;
		}
	}

	return status;
}

bool Tokenizer::tokenize(FILE* file, TokenStack* stack)
{
	bool status = true;
	
	m_file = file;
	m_stack = stack;
	m_buffer.reserve(256);
	m_input[0] = fgetc(m_file);
	m_input[1] = fgetc(m_file);

	char c = 0;

	while(status)
	{
		c = peek(0);
		if(IS_SPACE(c))
		{
			pop();
		}
		else if(IS_ALPHA(c))
		{
			status = read_identifier();
		}
		else if(IS_NUM(c) || (c == '\'') || (c == '"'))
		{
			status = read_literal();
		}
		else
		{
			char lookahead = peek(1);
			if((c == '/') && (lookahead == '/'))
			{
				status = read_single_line_comment();
			}
			else if((c == '/') && (lookahead == '*'))
			{
				status = read_multi_line_comment();
			}
			else
			{
				status = read_punctuator();
			}
		}
		
	}

	return status;
}

char Tokenizer::pop()
{
	char c = m_input[0];
	m_input[0] = m_input[1];
	m_input[1] = fgetc(m_file);
	return c;
}

char Tokenizer::peek(unsigned int offset)
{
	assert(offset <= 1);
	return m_input[offset];
}

bool Tokenizer::Tokenize(const char* file_path, TokenStack* stack)
{
	bool status = true;

	FILE* file = fopen(file_path, "r");
	if(file == nullptr)
	{
		status = false;
		printf("error: could not open file \"%s\" for reading\n", file_path);
	}

	if(status)
	{
		Tokenizer tokenizer;
		status = tokenizer.tokenize(file, stack);
	}

	if(file != nullptr)
	{
		fclose(file);
	}

	return status;
}

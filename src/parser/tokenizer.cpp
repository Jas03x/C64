#include <tokenizer.hpp>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ascii.hpp>

#define _strncmp(s0, s1, len) (strncmp((s0), (s1), (len)) == 0)

Tokenizer::Tokenizer()
{
	m_file = nullptr;
	m_stack = nullptr;
}

int Tokenizer::read_escape_character()
{
	int value = 0;
	bool status = expect('\\');

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
		Token tk = { TK_LITERAL, { .literal = { LITERAL_CHAR, { .character = c }}}};
		m_stack->push(tk);

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
		c = peek(0);
		if(c == '\\')
		{
			int value = read_escape_character();
			status = (value != -1);

			if(status) {
				c = (char) value;
			}
		}
		else
		{
			pop();
		}

		if(status)
		{
			if(c == '"') {
				break;
			} else {
				m_buffer.push_back(c);
			}
		}
	}

	if(status)
	{
		unsigned int len = m_buffer.size();
		
		char* str = new char[len + 1];
		memcpy(str, m_buffer.data(), len);
		str[len] = 0; // null terminate

		Token tk = { TK_LITERAL, { .literal = { LITERAL_STRING, { .string = { str, len } } } }};
		m_stack->push(tk);
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
		if(IS_NUM(c) || (c == '.'))
		{
			pop();
			if(c == '.')
			{
				if(!is_float) {
					is_float = true;
				} else {
					status = false;
					printf("error: repeated decimal symbol\n");
				}
			}
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
		Token tk = { TK_LITERAL, { .literal = { LITERAL_INTEGER, { .integer = strtoul(m_buffer.data(), nullptr, 10)}}}};

		m_stack->push(tk);
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
		Token tk = { TK_LITERAL, { .literal = { LITERAL_INTEGER, { .integer = strtoul(m_buffer.data(), nullptr, 16)}}}};

		m_stack->push(tk);
		m_buffer.clear();
	}

	return status;
}

bool Tokenizer::read_literal()
{
	bool status = true;

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
		c = peek(0);
		if(IS_ALPHA_NUM(c))
		{
			pop();
			m_buffer.push_back(c);
		}
		else
		{
			break;
		}
	}

	if(status && (m_buffer.size() == 0))
	{
		status = false;
		printf("error: expected identifier\n");
	}

	if(status)
	{
		Token tk = { 0, 0 };

		m_buffer.push_back(0);
		const char* str = m_buffer.data();
		unsigned int len = m_buffer.size();

		switch(len)
		{
			case 2:
			{
				switch(str[0])
				{
					case 'i': { if(_strncmp(str, "if", 2)) { tk = { TK_IF, { 0 } }; } break; }
					case 'o': { if(_strncmp(str, "or", 2)) { tk = { TK_OR, { 0 } }; } break; }
					case 'I':
					{
						switch(str[1])
						{
							case '8': { tk = { TK_TYPE, TK_TYPE_I8 }; break; }
						}
						break;
					}
					case 'U':
					{
						switch(str[1])
						{
							case '8': { tk = { TK_TYPE, TK_TYPE_U8 }; break; }
						}
						break;
					}
				}
				break;
			}
			case 3:
			{
				switch(str[0])
				{
					case 'a': { if(_strncmp(str, "and", 3)) { tk = { TK_AND, { 0 } }; } break; }
					case 'f': { if(_strncmp(str, "for", 3)) { tk = { TK_FOR, { 0 } }; } break; }
					case 'F':
					{
						switch(str[1])
						{
							case '3': { if(_strncmp(str, "F32", 3)) { tk = { TK_TYPE, TK_TYPE_F32 }; } break; }
							case '6': { if(_strncmp(str, "F64", 3)) { tk = { TK_TYPE, TK_TYPE_F64 }; } break; }
						}
						break;
					}
					case 'I':
					{
						switch(str[1])
						{
							case '1': { if(_strncmp(str, "I16", 3)) { tk = { TK_TYPE, TK_TYPE_I16 }; } break; }
							case '3': { if(_strncmp(str, "I32", 3)) { tk = { TK_TYPE, TK_TYPE_I32 }; } break; }
							case '6': { if(_strncmp(str, "I64", 3)) { tk = { TK_TYPE, TK_TYPE_I64 }; } break; }
						}
						break;
					}
					case 'U':
					{
						switch(str[1])
						{
							case '1': { if(_strncmp(str, "U16", 3)) { tk = { TK_TYPE, TK_TYPE_U16 }; } break; }
							case '3': { if(_strncmp(str, "U32", 3)) { tk = { TK_TYPE, TK_TYPE_U32 }; } break; }
							case '6': { if(_strncmp(str, "U64", 3)) { tk = { TK_TYPE, TK_TYPE_U64 }; } break; }
						}
						break;
					}
				}
				break;
			}
			case 4:
			{
				switch(str[0])
				{
					case 'c':
					{
						switch(str[3])
						{
							case 'e': { if(_strncmp(str, "case", 4)) { tk = { TK_CASE, { 0 } }; } break; }
							case 't': { if(_strncmp(str, "cast", 4)) { tk = { TK_STATIC_CAST, { 0 } }; } break; }
						break;
						}
						break;
					}
					case 'e':
					{
						switch(str[1])
						{
							case 'l': { if(_strncmp(str, "else", 4)) { tk = { TK_ELSE, { 0 } }; } break; }
							case 'n': { if(_strncmp(str, "enum", 4)) { tk = { TK_ENUM, { 0 } }; } break; }
						}
						break;
					}
					case 'v': { if(_strncmp(str, "void", 4)) { tk = { TK_TYPE, TK_TYPE_VOID }; } break; }
				}
				break;
			}
			case 5:
			{
				switch(str[0])
				{
					case 'c': { if(_strncmp(str, "const", 5)) { tk = { TK_CONST, { 0 } }; } break; }
					case 'w': { if(_strncmp(str, "while", 5)) { tk = { TK_WHILE, { 0 } }; } break; }
					case 'u': { if(_strncmp(str, "union", 5)) { tk = { TK_UNION, { 0 } }; } break; }
				}
				break;
			}
			case 6:
			{
				switch(str[0])
				{
					case 'e': { if(_strncmp(str, "extern", 6)) { tk = { TK_EXTERN, { 0 } }; } break; }
					case 'r':
					{
						switch(str[1])
						{
							case 'e': { if(_strncmp(str, "return", 6)) { tk = { TK_RETURN, { 0 } }; } break; }
							case '_': { if(_strncmp(str, "r_cast", 6)) { tk = { TK_REINTERPRET_CAST, { 0 } }; } break; }
						}
						break;
					}
					case 's': { if(_strncmp(str, "switch", 6)) { tk = { TK_SWITCH, { 0 } }; } break; }
				}
				break;
			}
			case 7:
			{
				switch(str[0])
				{
					case 'd': { if(_strncmp(str, "default", 7)) { tk = { TK_DEFAULT, { 0 } }; } break; }
				}
				break;
			}
			case 8:
			{
				switch(str[0])
				{
					case 'c': { if(_strncmp(str, "continue", 8)) { tk = { TK_CONTINUE, { 0 } }; } break; }
				}
				break;
			}
			case 9:
			{
				switch(str[0])
				{
					case 'n': { if(_strncmp(str, "namespace", 9)) { tk = { TK_NAMESPACE, { 0 } }; } break; }
				}
				break;
			}
		}
	
		if(tk.type == TK_INVALID) // this is an identifier
		{
			const char* id = m_stack->find_identifier(strptr(str, len));
			if(id == nullptr)
			{
				char* ptr = new char[len + 1];
				memcpy(ptr, str, len);
				ptr[len] = 0; // null terminate

				id = ptr;
				m_stack->insert_identifier(strptr(ptr, len));
			}
			

			tk.type = TK_IDENTIFIER;
			tk.data.identifier.len = len;
			tk.data.identifier.ptr = id;
		}

		if(status)
		{
			m_stack->push(tk);
			m_buffer.clear();
		}
	}

	return true;
}

bool Tokenizer::read_punctuator()
{
	bool status =  true;

	Token tk = { 0, 0 };
	char c = pop();
	
	switch(c)
	{
		case '=': { tk = { TK_EQUAL, { 0 } }; break; }
		case '<': { tk = { TK_LEFT_ARROW_HEAD, { 0 } };  break; }
		case '>': { tk = { TK_RIGHT_ARROW_HEAD, { 0 } }; break; }
		case '+': { tk = { TK_PLUS, { 0 } }; break; }
		case '-': { tk = { TK_MINUS, { 0 } }; break; }
		case '.': { tk = { TK_DOT, { 0 } }; break; }
		case '*': { tk = { TK_ASTERISK, { 0 } }; break; }
		case '/': { tk = { TK_FORWARD_SLASH, { 0 } }; break; }
		case '{': { tk = { TK_OPEN_CURLY_BRACKET, { 0 } }; break; }
		case '}': { tk = { TK_CLOSE_CURLY_BRACKET, { 0 } }; break; }
		case '(': { tk = { TK_OPEN_ROUND_BRACKET, { 0 } }; break; }
		case ')': { tk = { TK_CLOSE_ROUND_BRACKET, { 0 } }; break; }
		case '[': { tk = { TK_OPEN_SQUARE_BRACKET, { 0 } }; break; }
		case ']': { tk = { TK_CLOSE_SQUARE_BRACKET, { 0 } }; break; }
		case ';': { tk = { TK_SEMICOLON, { 0 } }; break; }
		case ',': { tk = { TK_COMMA, { 0 } }; break; }
		case '^': { tk = { TK_CARET, { 0 } }; break; }
		case '!': { tk = { TK_EXPLANATION_MARK, { 0 } }; break; }
		case '&': { tk = { TK_AMPERSAND, { 0 } }; break; }
		case '|': { tk = { TK_VERTICAL_BAR, { 0 } }; break; }
		case '%': { tk = { TK_PERCENT, { 0 } }; break; }
		case ':': { tk = { TK_COLON, { 0 } }; break; }
		default:
		{
			status = false;
			printf("unknown token '%c'\n", c);
			break;
		}
	}

	if(status)
	{
		m_stack->push(tk);
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
			if(c == EOF)
			{
				break;
			}
			else
			{
				if(c == '/')
				{
					switch(peek(1))
					{
						case '/': { status = read_single_line_comment(); break; }
						case '*': { status = read_multi_line_comment();  break; }
						default:  { status = read_punctuator(); break; }
					}
				}
				else
				{
					status = read_punctuator();
				}
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

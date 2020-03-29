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
			case 'x':
			{
				char s[3] = { 0 };
				s[0] = pop();
				s[1] = pop();

				if(IS_HEXADECIMAL(s[0]) && IS_HEXADECIMAL(s[1]))
				{
					value = (int) strtoul(s, nullptr, 16);
				}
				else
				{
					status = false;
					printf("error: expected hexadecimal character\n");
				}
				break;
			}
			default:
			{
				status = false;
				printf("error: unknown escape sequence");
				break;
			}
		}
	}

	return status ? value : -1;
}

bool Tokenizer::expect(char c)
{
	bool status = true;

	if(pop() != c)
	{
		status = false;
		printf("error: expected \"%c\"\n", c);
	}

	return status;
}

bool Tokenizer::read_character_literal()
{
	bool status = expect('\'');

	char c = 0;
	if(status)
	{
		if(m_input[0] != '\\')
		{
			c = pop();
		}
		else
		{
			int value = read_escape_character();
			if(value != -1)
			{
				c = (char) value;
			}
			else
			{
				status = false;
			}
		}
	}

	if(status)
	{
		Token tk = { 0, { 0 }};
		tk.type = TK_LITERAL;
		tk.data.literal.type = LITERAL_CHAR;
		tk.data.literal.data.character = c;

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
		if(m_input[0] != '\\')
		{
			c = pop();
		}
		else
		{
			int value = read_escape_character();
			if(value != -1)
			{
				c = (char) value;
			}
			else
			{
				status = false;
			}
		}

		if(status)
		{
			if(c == '"')
			{
				break;
			}
			else
			{
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

		Token tk = { 0, { 0 }};
		tk.type = TK_LITERAL;
		tk.data.literal.type = LITERAL_STRING;
		tk.data.literal.data.string.ptr = str;
		tk.data.literal.data.string.len = len;

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
		c = m_input[0];
		if(IS_NUM(c) || (c == '.'))
		{
			pop();
			if(c == '.')
			{
				if(!is_float)
				{
					is_float = true;
				}
				else
				{
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

		Token tk = { TK_LITERAL, { 0 }};
		if(!is_float)
		{
			tk.data.literal.type = LITERAL_INTEGER;
			tk.data.literal.data.integer_value = strtoul(m_buffer.data(), nullptr, 10);
		}
		else
		{
			tk.data.literal.type = LITERAL_FLOAT;
			tk.data.literal.data.float_value = strtof(m_buffer.data(), nullptr);
		}

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
		c = m_input[0];
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
		
		Token tk = { 0, { 0 } };
		tk.type = TK_LITERAL;
		tk.data.literal.type = LITERAL_INTEGER;
		tk.data.literal.data.integer_value = strtoul(m_buffer.data(), nullptr, 16);

		m_stack->push(tk);
		m_buffer.clear();
	}

	return status;
}

bool Tokenizer::read_literal()
{
	bool status = true;

	char c = m_input[0];
	if(c == '\'')
	{
		status = read_character_literal();
	}
	else if(c == '"')
	{
		status = read_string();
	}
	else if((c == '0') && (m_input[1]== 'x'))
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
	bool status = expect('/') ? expect('/') : false;

	char c = 0;
	while(status)
	{
		c = pop();
		if((c == '\n') || (c == EOF))
		{
			break;
		}
	}

	return status;
}

bool Tokenizer::read_multi_line_comment()
{
	bool status = expect('/') ? expect('*') : false;

	char c = 0;
	while(status)
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

	char c = 0;
	while(status)
	{
		c = m_input[0];
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
		Token tk = { 0, { 0 }};

		m_buffer.push_back(0);
		const char* str = m_buffer.data();
		unsigned int len = m_buffer.size();

		switch(len)
		{
			case 2:
			{
				switch(str[0])
				{
					case 'i': { if(_strncmp(str, "if", 2)) { tk.type = TK_IF; } break; }
					case 'o': { if(_strncmp(str, "or", 2)) { tk.type = TK_OR; } break; }
					case 'I':
					{
						switch(str[1])
						{
							case '8': { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_I8; break; }
						}
						break;
					}
					case 'U':
					{
						switch(str[1])
						{
							case '8': { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_U8; break; }
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
					case 'a': { if(_strncmp(str, "and", 3)) { tk.type = TK_AND; } break; }
					case 'f': { if(_strncmp(str, "for", 3)) { tk.type = TK_FOR; } break; }
					case 'F':
					{
						switch(str[1])
						{
							case '3': { if(_strncmp(str, "F32", 3)) { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_F32; } break; }
							case '6': { if(_strncmp(str, "F64", 3)) { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_F64; } break; }
						}
						break;
					}
					case 'I':
					{
						switch(str[1])
						{
							case '1': { if(_strncmp(str, "I16", 3)) { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_I16; } break; }
							case '3': { if(_strncmp(str, "I32", 3)) { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_I32; } break; }
							case '6': { if(_strncmp(str, "I64", 3)) { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_I64; } break; }
						}
						break;
					}
					case 'U':
					{
						switch(str[1])
						{
							case '1': { if(_strncmp(str, "U16", 3)) { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_U16; } break; }
							case '3': { if(_strncmp(str, "U32", 3)) { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_U32; } break; }
							case '6': { if(_strncmp(str, "U64", 3)) { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_U64; } break; }
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
							case 'e': { if(_strncmp(str, "case", 4)) { tk.type = TK_CASE; } break; }
							case 't': { if(_strncmp(str, "cast", 4)) { tk.type = TK_STATIC_CAST; } break; }
						}
						break;
					}
					case 'e':
					{
						switch(str[1])
						{
							case 'l': { if(_strncmp(str, "else", 4)) { tk.type = TK_ELSE; } break; }
							case 'n': { if(_strncmp(str, "enum", 4)) { tk.type = TK_ENUM; } break; }
						}
						break;
					}
					case 'v': { if(_strncmp(str, "void", 4)) { tk.type = TK_TYPE; tk.data.subtype = TK_TYPE_VOID; } break; }
				}
				break;
			}
			case 5:
			{
				switch(str[0])
				{
					case 'c': { if(_strncmp(str, "const", 5)) { tk.type = TK_CONST; } break; }
					case 'w': { if(_strncmp(str, "while", 5)) { tk.type = TK_WHILE; } break; }
					case 'u': { if(_strncmp(str, "union", 5)) { tk.type = TK_UNION; } break; }
				}
				break;
			}
			case 6:
			{
				switch(str[0])
				{
					case 'e': { if(_strncmp(str, "extern", 6)) { tk.type = TK_EXTERN; } break; }
					case 'r':
					{
						switch(str[1])
						{
							case 'e': { if(_strncmp(str, "return", 6)) { tk.type = TK_RETURN; } break; }
							case '_': { if(_strncmp(str, "r_cast", 6)) { tk.type = TK_REINTERPRET_CAST; } break; }
						}
						break;
					}
					case 's': { if(_strncmp(str, "switch", 6)) { tk.type = TK_SWITCH; } break; }
				}
				break;
			}
			case 7:
			{
				switch(str[0])
				{
					case 'd': { if(_strncmp(str, "default", 7)) { tk.type = TK_DEFAULT; } break; }
				}
				break;
			}
			case 8:
			{
				switch(str[0])
				{
					case 'c': { if(_strncmp(str, "continue", 8)) { tk.type = TK_CONTINUE; } break; }
				}
				break;
			}
		}
	
		if(tk.type == TK_INVALID) // this is an identifier
		{
			const char* id = m_stack->find_identifier(strptr(str, len));
			if(id == nullptr)
			{
				char* ptr = new char[len];
				memcpy(ptr, str, len); // 'str' is already null terminated
				
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

	Token tk = { 0 };
	char c = pop();
	
	switch(c)
	{
		case '=': { tk.type = TK_EQUAL; break; }
		case '<': { tk.type = TK_LEFT_ARROW_HEAD; break; }
		case '>': { tk.type = TK_RIGHT_ARROW_HEAD; break; }
		case '+': { tk.type = TK_PLUS; break; }
		case '-': { tk.type = TK_MINUS; break; }
		case '.': { tk.type = TK_DOT; break; }
		case '*': { tk.type = TK_ASTERISK; break; }
		case '/': { tk.type = TK_FORWARD_SLASH; break; }
		case '{': { tk.type = TK_OPEN_CURLY_BRACKET; break; }
		case '}': { tk.type = TK_CLOSE_CURLY_BRACKET; break; }
		case '(': { tk.type = TK_OPEN_ROUND_BRACKET; break; }
		case ')': { tk.type = TK_CLOSE_ROUND_BRACKET; break; }
		case '[': { tk.type = TK_OPEN_SQUARE_BRACKET; break; }
		case ']': { tk.type = TK_CLOSE_SQUARE_BRACKET; break; }
		case ';': { tk.type = TK_SEMICOLON; break; }
		case ',': { tk.type = TK_COMMA; break; }
		case '^': { tk.type = TK_CARET; break; }
		case '!': { tk.type = TK_EXPLANATION_MARK; break; }
		case '&': { tk.type = TK_AMPERSAND; break; }
		case '|': { tk.type = TK_VERTICAL_BAR; break; }
		case '%': { tk.type = TK_PERCENT; break; }
		case ':': { tk.type = TK_COLON; break; }
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
		c = m_input[0];
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
					switch(m_input[1])
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

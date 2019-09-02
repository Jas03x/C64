#include <tokenizer.hpp>

#include <string.h>

#include <string>
#include <vector>

#include <ascii.hpp>
#include <file.hpp>

#define _strncmp(str0, str1, num) (strncmp((str0), (str1), (num)) == 0)

inline bool is_num(char c) {
	return (c >= '0') && (c <= '9');
}

inline bool is_alpha(char c) {
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}

inline bool is_alpha_or_num(char c) {
	return is_alpha(c) || is_num(c);
}

inline bool is_hex(char c) {
	return is_num(c) || (((c >= 'a') && (c <= 'f')) && ((c >= 'A') && (c <= 'F')));
}

bool Tokenizer::Tokenize(const char* path, TokenStack& stack)
{
	bool status = Process(path, stack);
	if (status)
	{
		Token eof = { TK_EOF };
		stack.push(eof);
	}
	return status;
}

bool Tokenizer::Process(const char* path, TokenStack& stack)
{
	bool status = true;

	unsigned int source_size = 0;
	char* source = File::Read(path, source_size);

	status = source != nullptr;
	if (status)
	{
		Tokenizer tokenizer;
		status = tokenizer.process(source, source_size, &stack);
	}

	delete[] source;
	return status;
}

Tokenizer::Tokenizer()
{
	m_source = nullptr;
	m_source_size = 0;
	m_read_ptr = 0;
	m_stack = nullptr;
}

char Tokenizer::pop()
{
	char c = 0;
	if (m_read_ptr < m_source_size)
	{
		c = m_source[m_read_ptr++];
	}
	return c;
}

char Tokenizer::peek(unsigned int offset)
{
	char c = 0;
	if (m_read_ptr + offset < m_source_size)
	{
		c = m_source[m_read_ptr + offset];
	}
	return c;
}

const char* Tokenizer::current_position()
{
	return &m_source[m_read_ptr];
}

bool Tokenizer::process(const char* source, unsigned int source_size, TokenStack* stack)
{
	bool status = true;

	m_source = source;
	m_source_size = source_size;
	m_read_ptr = 0;
	m_stack = stack;

	while (status)
	{
		char c = peek(0);
		if (c == 0)
		{
			break;
		}

		switch (c)
		{
			case ' ': case '\n': case '\t':
			{
				pop();
				continue;
			}

			case '/':
			{
				if (peek(1) == '/') status = read_single_line_comment();
				else if (peek(1) == '*') status = read_multi_line_comment();
				else goto DEFAULT;
				break;
			}

			case '#':
			{
				status = read_preprocessor();
				break;
			}

			DEFAULT: default:
			{
				status = read_token();
				break;
			}
		}
	}

	return status;
}

bool Tokenizer::read_token()
{
	bool status = true;

	char c = peek(0);
	switch (c)
	{
		FORWARD_SLASH:
		case '+': case '-': case '*': case ':':
		case '=': case '<': case '>': case ';':
		case '(': case ')': case '[': case ']':
		case '{': case '}': case ',': case '.':
		case '&': case '!':
		{
			pop();
			status = process(c);
			break;
		}

		case '\'':
		{
			status = read_character();
		}

		default:
		{
			if (is_num(c) || (c == '"'))
			{
				status = read_literal();
			}
			else if (is_alpha(c) || (c == '_'))
			{
				status = read_identifier();
			}
			else
			{
				status = false;
				printf("Error: Unknown token '%c'\n", c);
			}
			break;
		}
	}

	return status;
}

bool Tokenizer::process(char c)
{
    bool ret = true;

	Token tk = {};
    switch(c)
    {
        case '<': { tk.type = TK_LEFT_ARROW_HEAD;      break; }
        case '>': { tk.type = TK_RIGHT_ARROW_HEAD;     break; }
        case '=': { tk.type = TK_EQUAL;                break; }
        case '+': { tk.type = TK_PLUS;                 break; }
        case '-': { tk.type = TK_MINUS;                break; }
        case '*': { tk.type = TK_ASTERISK;             break; }
        case '^': { tk.type = TK_CARET;                break; }
        case '&': { tk.type = TK_AMPERSAND;            break; }
        case '!': { tk.type = TK_EXPLANATION_MARK;     break; }
        case '.': { tk.type = TK_DOT;                  break; }
        case '/': { tk.type = TK_FORWARD_SLASH;        break; }
        case '{': { tk.type = TK_OPEN_CURLY_BRACKET;   break; }
        case '}': { tk.type = TK_CLOSE_CURLY_BRACKET;  break; }
        case '(': { tk.type = TK_OPEN_ROUND_BRACKET;   break; }
        case ')': { tk.type = TK_CLOSE_ROUND_BRACKET;  break; }
        case '[': { tk.type = TK_OPEN_SQUARE_BRACKET;  break; }
        case ']': { tk.type = TK_CLOSE_SQUARE_BRACKET; break; }
        case ';': { tk.type = TK_SEMICOLON;            break; }
        case ',': { tk.type = TK_COMMA;                break; }
        case '%': { tk.type = TK_PERCENT;              break; }
		case ':': { tk.type = TK_COLON;                break; }

        default:
        {
			ret = false;
            printf("Error: Unknown character '%c'\n", c);
            break;
        }
    }

    printf("%c => %hhu\n", c, tk.type);

	if (ret)
	{
		m_stack->push(tk);
	}

    return ret;
}

bool Tokenizer::process(const char* str, unsigned int len)
{
    bool ret = true;

	Token tk = {};
    switch(len)
    {
        case 2:
        {
            switch(str[0])
            {
                case 'i': { if(_strncmp(str, "if", 2)) { tk.type = TK_IF;                  } break; }
                case 'o': { if(_strncmp(str, "or", 2)) { tk.type = TK_OR;                  } break; }
                case 'I': { if(_strncmp(str, "I8", 2)) { tk = { TK_TYPE, { TK_TYPE_I8 } }; } break; }
                case 'U': { if(_strncmp(str, "U8", 2)) { tk = { TK_TYPE, { TK_TYPE_U8 } }; } break; }
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
                        case '3': { if(_strncmp(str, "F32", 3)) { tk = { TK_TYPE, { TK_TYPE_F32 } }; } break; }
                        case '6': { if(_strncmp(str, "F64", 3)) { tk = { TK_TYPE, { TK_TYPE_F64 } }; } break; }
                    }
                    break;
                }
                case 'I':
                {
                    switch(str[1])
                    {
                        case '1': { if(_strncmp(str, "I16", 3)) { tk = { TK_TYPE, { TK_TYPE_I16 } }; } break; }
                        case '3': { if(_strncmp(str, "I32", 3)) { tk = { TK_TYPE, { TK_TYPE_I32 } }; } break; }
                        case '6': { if(_strncmp(str, "I64", 3)) { tk = { TK_TYPE, { TK_TYPE_I64 } }; } break; }
                    }
                    break;
                }
                case 'U':
                {
                    switch(str[1])
                    {
                        case '1': { if(_strncmp(str, "U16", 3)) { tk = { TK_TYPE, { TK_TYPE_U16 } }; } break; }
                        case '3': { if(_strncmp(str, "U32", 3)) { tk = { TK_TYPE, { TK_TYPE_U32 } }; } break; }
                        case '6': { if(_strncmp(str, "U64", 3)) { tk = { TK_TYPE, { TK_TYPE_U64 } }; } break; }
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
                        case 'a': { if(_strncmp(str, "case", 4)) { tk.type = TK_CASE;        } break; }
                        case 't': { if(_strncmp(str, "cast", 4)) { tk.type = TK_STATIC_CAST; } break; }
                    }
                }
                case 'g': { if(_strncmp(str, "goto", 4)) { tk.type = TK_GOTO;                  } break; }
                case 'v': { if(_strncmp(str, "void", 4)) { tk = { TK_TYPE, { TK_TYPE_VOID } }; } break; }
                case 'e':
                {
                    switch(str[1])
                    {
                        case 'l': { if(_strncmp(str, "else", 4)) { tk.type = TK_ELSE; } break; }
                        case 'n': { if(_strncmp(str, "enum", 4)) { tk.type = TK_ENUM; } break; }
                    }
                    break;
                }
            }
            break;
        }

        case 5:
        {
            switch(str[0])
            {
                case 'b': { if(_strncmp(str, "break", 5)) { tk.type = TK_BREAK; } break; }
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
                case 'e':
                {
                    switch(str[2])
                    {
                        case 't': { if(_strncmp(str, "extern", 6)) { tk.type = TK_EXTERN; } break; }
                    }
                }
                case 'r':
                {
                    switch(str[1])
                    {
                        case 'e': { if(_strncmp(str, "return", 6)) { tk.type = TK_RETURN;           } break; }
                        case '_': { if(_strncmp(str, "r_cast", 6)) { tk.type = TK_REINTERPRET_CAST; } break; }
                    }
                }
                case 's':
                {
                    switch(str[1])
                    {
                        case 't': { if(_strncmp(str, "struct", 6)) { tk.type = TK_STRUCT; } break; }
                        case 'w': { if(_strncmp(str, "switch", 6)) { tk.type = TK_SWITCH; } break; }
                    }
                    break;
                }
            }
            break;
        }

        case 7:
        {
            switch(str[0])
            {
                case 'd': { if(_strncmp(str, "default", 7)) { tk.type = TK_DEFAULT; } break; }
                case 't': { if(_strncmp(str, "typedef", 7)) { tk.type = TK_TYPEDEF; } break; }
            }
            break;
        }

        case 8:
        {
            if(_strncmp(str, "continue", 8)) { tk.type = TK_CONTINUE; }
            break;
        }

        case 9:
        {
            if(_strncmp(str, "namespace", 9)) { tk.type = TK_NAMESPACE; }
            break;
        }
    }

    if(tk.type == TK_INVALID)
    {
		char* str_ptr = reinterpret_cast<char*>(malloc(len + 1));
		strncpy(str_ptr, str, len);
		str_ptr[len] = 0;

        tk.type = TK_IDENTIFIER;
		tk.identifier.string.ptr = str_ptr;
        tk.identifier.string.len = len;
    }

	if (ret)
	{
		m_stack->push(tk);
	}

    printf("%.*s => %hhu\n", len, str, tk.type);
    return ret;
}

bool Tokenizer::read_identifier()
{
	const char* str = nullptr;
	unsigned int len = 0;

	bool status = read_word(str, len);
	if (status)
	{
		status = process(str, len);
	}

	return status;
}

bool Tokenizer::read_word(const char*& str_ptr, unsigned int& str_len)
{
	bool status = true;

	const char* str = current_position();
	unsigned int len = 0;

	while (status)
	{
		char c = peek(0);
		if (is_alpha_or_num(c) || c == '_')
		{
			pop();
			len++;
		}
		else
		{
			break;
		}
	}

	if (status)
	{
		str_ptr = str;
		str_len = len;
	}

	return status;
}

bool Tokenizer::read_single_line_comment()
{
    bool ret = true;

    while(true)
    {
        char c = pop();
        if((c == 0) || (c == '\n')) { break; }
    }

    return ret;
}

bool Tokenizer::read_multi_line_comment()
{
    bool ret = true;

    while(true)
    {
        char c = pop();
        if(c == 0)
        {
			ret = false;
			printf("unterminated multi-line comment\n");
            break;
        }
        else if(c == '*')
        {
            c = pop();
            if(c == '/')
            {
                break;
            }
        }
    }

    return ret;
}

bool Tokenizer::read_literal()
{
	bool ret = true;

	Token tk = {};

	char c = peek(0);
	if (c == '"') {
		ret = read_string(tk);
	}
	else {
		ret = read_value(tk);
	}

	if (ret) {
		m_stack->push(tk);
	}

	return ret;
}

bool Tokenizer::read_string(Token& tk)
{
	bool status = (pop() == '"');

	m_buffer.clear();
	while (status)
	{
		char c = pop();
		if (c == 0)
		{
			printf("Error: Unexpected EOF\n");
			status = false;
			break;
		}
		else if (c == '"')
		{
			break;
		}
		else
		{
			if (c == '\\') {
				status = read_escape_character(c);
			}

			if (status) {
				m_buffer.push_back(c);
			}
		}
	}

	m_buffer.push_back(0); // insert the null terminator

	if (status)
	{
		tk.type = TK_LITERAL;
		tk.literal.type = LITERAL_STRING;
		tk.literal.string.ptr = strdup(m_buffer.data());
		tk.literal.string.len = m_buffer.size();

		printf("%.*s => %hhu\n", tk.literal.string.len, tk.literal.string.ptr, tk.type);
	}

	return status;
}

bool Tokenizer::read_character()
{
	bool status = true;

	pop(); // pop the '

	char c = pop();
	if (c == '\\')
	{
		status = read_escape_character(c);
	}

	if (pop() != '\'')
	{
		status = false;
		printf("invalid character sequence\n");
	}

	if (status)
	{
		Token tk = {};
		tk.type = TK_LITERAL;
		tk.literal.type = LITERAL_CHAR;
		tk.literal.character = c;

		m_stack->push(tk);
	}

	return status;
}

bool Tokenizer::read_hex_value(Token& tk)
{  
    const char* ptr = current_position();
    unsigned int len = 0;

    bool ret = (pop() == '0') && (pop() == 'x');
    if(ret)
    {
        while(true)
        {
            char c = pop();
            if(is_hex(c))
            {
                len ++;
            }
            else
            {
                break;
            }
        }
    }

    if(ret)
    {
        tk.type = TK_LITERAL;
        tk.literal.type = LITERAL_INTEGER;
        tk.literal.integer.value = strtoul(ptr, nullptr, 16);
    }

    printf("%.*s => %hhu %hhu\n", len, ptr, tk.type, tk.literal.type);

    return ret;
}

bool Tokenizer::read_dec_value(Token& tk)
{
    bool ret = true;

    const char* ptr = current_position();
    unsigned int len = 0;

    bool is_decimal = false;
    while(ret)
    {
        char c = peek(0);
        if(is_num(c))
        {
            pop();
            len ++;
        }
        else if(c == '.')
        {
            pop();
            len ++;

            if(!is_decimal) {
				is_decimal = true;
			} else {
				ret = false;
                printf("error: repeated decimal place\n");
                break;
            }
        }
        else
		{
            break;
        }
    }

    if(ret)
    {
        tk.type = TK_LITERAL;

        if(is_decimal)
        {
            tk.literal.type = LITERAL_DECIMAL;
            tk.literal.decimal.value = strtod(ptr, nullptr);
            
            printf("%.*s => %hhu %hhu\n", len, ptr, tk.type, tk.literal.type);
        }
        else
        {
            tk.literal.type = LITERAL_INTEGER;
            tk.literal.integer.value = strtoul(ptr, nullptr, 10);
            
            printf("%.*s => %hhu %hhu\n", len, ptr, tk.type, tk.literal.type);
        }
    }

    return ret;
}

bool Tokenizer::read_value(Token &tk)
{
    bool ret = true;

    if((peek(0) == '0') && (peek(1) == 'x'))
    {
        ret = read_hex_value(tk);
    }
    else
    {
        ret = read_dec_value(tk);
    }

    return ret;
}

bool Tokenizer::skip_spaces()
{
	bool status = true;

	while (status)
	{
		char c = peek(0);
		if (c == 0)
		{
			status = false;
			printf("unexpected eof\n");
		}
		else if ((c == ' ') || (c == '\t'))
		{
			pop();
			continue;
		}
		else
		{
			break;
		}
	}

	return status;
}

bool Tokenizer::read_pp_include()
{
    bool status = true;

	bool is_relative = false;

	status = skip_spaces();
	if (status)
	{
		char c = pop();
		if (c == '<') { }
		else if (c == '"') { is_relative = true; }
		else
		{
			status = false;
			printf("invalid include preprocessor\n");
		}
	}

    const char* ptr = current_position();
    unsigned int len = 0;

    while(status)
    {
        char c = pop();
        
		if (c == '>' || c == '"')
		{
			if ((is_relative && (c == '>')) || (!is_relative && (c == '"')))
			{
				status = false;
				printf("invalid include preprocessor\n");
			}

			break;
		}
		else if(is_alpha_or_num(c) || c == '_' || c == '.')
		{
			len++;
		}
		else
		{
			status = false;
			printf("invalid include preprocessor\n");
		}
    }

	if (status)
	{
		status = skip_spaces();
	}

	if (status)
	{
		if (pop() != '\n')
		{
			status = false;
			printf("invalid include preprocessor\n");
		}
	}

	if (status)
	{
		char file_path[1024] = {};
		snprintf(file_path, 1024, "C:/Users/Jas/Documents/C64/test/%.*s", len, ptr);
		
		printf("INCLUDE: %s\n", file_path);
		status = Process(file_path, *m_stack);
	}

    return status;
}

bool Tokenizer::read_preprocessor()
{
    bool status = true;
    const char* ptr = nullptr;
    unsigned int len = 0;

    pop(); // pop the '#'

    status = read_word(ptr, len);
    if(status)
    {
        switch(len)
        {
            case 2: { if(_strncmp(ptr, "if",    2)) { } break; }
            case 4: { if(_strncmp(ptr, "elif",  4)) { } break; }
            case 5:
            {
                switch(ptr[0])
                {
                    case 'e': { if(_strncmp(ptr, "endif", 5)) { } break; }
                    case 'i': { if(_strncmp(ptr, "ifdef", 5)) { } break; }
                }
                break;
            }
            case 6:
            {
                switch(ptr[0])
                {
                    case 'd': { if(_strncmp(ptr, "define", 6)) { } break; }
                    case 'i': { if(_strncmp(ptr, "ifndef", 6)) { } break; }
                }
                break;
            }
            case 7: { if(_strncmp(ptr, "include", 7)) { status = read_pp_include(); } break; }
        }
    }

    if(status)
    {
    }

    return status;   
}

bool Tokenizer::read_escape_character(char& character)
{
	bool status = true;

	char c = pop();
	switch (c)
	{
		case 'n': { character = ASCII_NEWLINE;         break; }
		case 'r': { character = ASCII_CARRIAGE_RETURN; break; }
		case 't': { character = ASCII_TAB;             break; }
		case '"': { character = ASCII_DOUBLE_QUOTE;    break; }
		case '\'': { character = ASCII_SINGLE_QUOTE;    break; }
		case '\\': { character = ASCII_BACK_SLASH;      break; }

		case 'x':
		{
			char buf[3];
			buf[0] = pop();
			buf[1] = pop();
			buf[2] = 0;

			if (!is_alpha_or_num(buf[0]) || !is_alpha_or_num(buf[1]))
			{
				status = false;
				printf("invalid character in escape sequence\n");
			}

			if (status)
			{
				long value = strtol(buf, nullptr, 16);
				character = static_cast<char>(value);
			}

			break;
		}

		default:
		{
			status = false;
			printf("unknown character escape sequence\n");
		}
	}

	return status;
}

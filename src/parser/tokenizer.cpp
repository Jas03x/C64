#include <tokenizer.hpp>

#include <string>
#include <vector>

#include <ascii.hpp>

#include <status.hpp>

char Tokenizer::pop()
{
    char c = 0;
    if(m_read_ptr < m_buffer_size)
    {
        c = m_buffer[m_read_ptr++];
    }
    return c;
}

char Tokenizer::peek(unsigned int offset)
{
    char c = 0;
    if(m_read_ptr + offset < m_buffer_size)
    {
        c = m_buffer[m_read_ptr + offset];
    }
    return c;
}

const char* Tokenizer::current_ptr()
{
    return &m_buffer[m_read_ptr];
}

bool Tokenizer::read_single_line_comment()
{
    bool ret = true;

    while(true)
    {
        char c = pop();

        if(c == 0)
        {
            ret = false;
            break;
        }
        else if(c == '\n')
        {
            break;
        }
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

bool Tokenizer::read_hex_value(Token& tk)
{  
    const char* ptr = current_ptr();
    unsigned int len = 0;

    bool ret = (pop() == '0') && (pop() == 'x');
    
    if(ret)
    {
        while(true)
        {
            char c = pop();
            if(((c >= '0') && (c <= '9')) && ((c >= 'A') && (c <= 'F')))
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

    const char* ptr = current_ptr();
    unsigned int len = 0;

    bool is_decimal = false;
    
    while(true)
    {
        char c = peek(0);
        if((c >= '0') && (c <= '9'))
        {
            pop();
            len ++;
        }
        else if(c == '.')
        {
            pop();
            len ++;

            if(!is_decimal)
            {
                is_decimal = true;
            }
            else
            {
                printf("Error: Duplicated decimal operator\n");
                ret = false;
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

bool Tokenizer::read_value(Token& tk)
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

bool Tokenizer::read_literal(Token& tk)
{
    bool ret = true;

    char c = peek(0);
    if (c == '"')
    {
        ret = read_string(tk);
    }
    else
    {
        ret = read_value(tk);
    }

    return ret;
}

bool Tokenizer::read_string(Token& tk)
{
    bool status = (pop() == '"');

    m_string_buffer.clear();

    while(status)
    {
        char c = pop();
        if(c == 0)
        {
            printf("Error: Unexpected EOF\n");
            status = false;
            break;
        }
        else if(c == '"')
        {
            break;
        }
        else
        {
            if(c == '\\')
            {
                status = read_escape_character(c);
            }
            
            if(status)
            {
                m_string_buffer.push_back(c);
            }
        }
    }

    m_string_buffer.push_back(0); // insert the null terminator

    if(status)
    {
        tk.type = TK_LITERAL;
        tk.literal.type = LITERAL_STRING;
        tk.literal.string.ptr = strdup(m_string_buffer.data());
        tk.literal.string.len = m_string_buffer.size();

        printf("%.*s => %hhu\n", tk.literal.string.len, tk.literal.string.ptr, tk.type);
    }

    return status;
}

bool Tokenizer::read_escape_character(char& character)
{
    bool status = true;

    char c = pop();
    switch(c)
    {
        case 'n':  { character = ASCII_NEWLINE;         break; }
        case 'r':  { character = ASCII_CARRIAGE_RETURN; break; }
        case 't':  { character = ASCII_TAB;             break; }
        case '"':  { character = ASCII_DOUBLE_QUOTE;    break; }
        case '\'': { character = ASCII_SINGLE_QUOTE;    break; }
        case '\\': { character = ASCII_BACK_SLASH;      break; }

        case 'x':
        {
            char buf[3];
            for(unsigned int i = 0; i < 2; i++)
            {
                buf[i] = pop();
                if(!(((buf[i] >= '0') && (buf[i] <= '9')) || ((buf[i] >= 'A') && (buf[i] <= 'F'))))
                {
                    status = false;
                    printf("invalid character in escape sequence\n");
                }
            }
            buf[2] = 0;
            
            if(status)
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

bool Tokenizer::read_character(Token& tk)
{
    bool status = true;

    pop(); // pop the '

    char c = pop();
    if(c == '\\')
    {
        status = read_escape_character(c);
    }

    if(pop() != '\'')
    {
        status = false;
        printf("invalid character sequence\n");
    }

    if(status)
    {
        tk.type = TK_LITERAL;
        tk.literal.type = LITERAL_CHAR;
        tk.literal.character = c;
    }

    return status;
}

bool Tokenizer::read_token(Token& tk)
{
    bool ret = true;

    const char* ptr = current_ptr();
    unsigned int len = 0;

    while(true)
    {
        char c = peek(0);
        
        bool v = false;
        v |= ((c >= '0') && (c <= '9'));
        v |= ((c >= 'a') && (c <= 'z'));
        v |= ((c >= 'A') && (c <= 'Z'));
        
        if(!v) // check for any special characters
        {
            v = (c == '_');
        }

        if(v)
        {
            pop();
            len ++;
        }
        else
        {
            break;
        }
    }

    if(ret)
    {
        ret = process(ptr, len, tk);
    }
    
    return ret;
}

bool Tokenizer::process(char c, Token& tk)
{
    bool ret = true;

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
            printf("Error: Unknown character '%c'\n", c);
            ret = false;
            break;
        }
    }

    printf("%c => %hhu\n", c, tk.type);

    return ret;
}

#define _strncmp(str0, str1, num) (strncmp((str0), (str1), (num)) == 0)
bool Tokenizer::process(const char* str, unsigned int len, Token& tk)
{
    bool ret = true;

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
                default:  { break; }
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
                case 'c': { if(_strncmp(str, "case", 4)) { tk.type = TK_CASE;                  } break; }
                case 'g': { if(_strncmp(str, "goto", 4)) { tk.type = TK_GOTO;                  } break; }
                case 'v': { if(_strncmp(str, "VOID", 4)) { tk = { TK_TYPE, { TK_TYPE_VOID } }; } break; }
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
                        case 'p': { if(_strncmp(str, "export", 6)) { tk.type = TK_EXPORT; } break; }
                        case 't': { if(_strncmp(str, "extern", 6)) { tk.type = TK_EXTERN; } break; }
                    }
                }
                case 'i': { if(_strncmp(str, "import", 6)) { tk.type = TK_IMPORT; } break; }
                case 'm': { if(_strncmp(str, "module", 6)) { tk.type = TK_MODULE; } break; }
                case 'p': { if(_strncmp(str, "public", 6)) { tk.type = TK_PUBLIC; } break; }
                case 'r': { if(_strncmp(str, "return", 6)) { tk.type = TK_RETURN; } break; }
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
                case 'p': { if(_strncmp(str, "private", 7)) { tk.type = TK_PRIVATE; } break; }
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

		case 11:
		{
			if (_strncmp(str, "static_cast", 11)) { tk.type = TK_STATIC_CAST; }
			break;
		}

		case 16:
		{
			if (_strncmp(str, "reinterpret_cast", 16)) { tk.type = TK_REINTERPRET_CAST; }
			break;
		}
    }

    if(tk.type == TK_INVALID)
    {
        tk.type = TK_IDENTIFIER;
        tk.identifier.string.ptr = str;
        tk.identifier.string.len = len;
    }

    printf("%.*s => %hhu\n", len, str, tk.type);

    return ret;
}

bool Tokenizer::next_token(Token& tk)
{
    STATUS status = STATUS_WORKING;

    while(status == STATUS_WORKING)
    {
        char c = peek(0);

        switch(c)
        {
            case 0:
            {
                tk.type = TK_EOF;
                status = STATUS_SUCCESS;
                break;
            }

            case ' ': case '\n': case '\t':
            {
                pop();
                break;
            }

            case '/':
            {
                pop();

                char n = peek(0);
                if(n == '/')
                {
                    pop();
                    status = read_single_line_comment() ? STATUS_WORKING : STATUS_ERROR;
                }
                else if(n == '*')
                {
                    pop();
                    status = read_multi_line_comment() ? STATUS_WORKING : STATUS_ERROR;
                }
                else
                {
                    goto FORWARD_SLASH;
                }
                break;
            }

            FORWARD_SLASH:
            case '+': case '-': case '*': case ':':
            case '=': case '<': case '>': case ';':
            case '(': case ')': case '[': case ']':
            case '{': case '}': case ',': case '.':
			case '&': case '!':
            {
                pop();
                status = process(c, tk) ? STATUS_SUCCESS : STATUS_ERROR;
                break;
            }

            case '\'':
            {
                status = read_character(tk) ? STATUS_SUCCESS : STATUS_ERROR;
                break;
            }

            default:
            {
                if(((c >= '0') && (c <= '9')) || (c == '"'))
                {
                    status = read_literal(tk) ? STATUS_SUCCESS : STATUS_ERROR;
                }
                else if(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_'))
                {
                    status = read_token(tk) ? STATUS_SUCCESS : STATUS_ERROR;
                }
                else
                {
                    printf("Error: Unknown token '%c'\n", c);
                    status = STATUS_ERROR;
                }
                break;
            }
        }
    }

    return (status == STATUS_SUCCESS);
}

Tokenizer::Tokenizer(const char* buffer, long size)
{
    m_buffer = buffer;
    m_buffer_size = size;

    m_read_ptr = 0;
}

bool Tokenizer::tokenize()
{
    while(true)
    {
        Token tk = { 0 };
        if(next_token(tk))
        {
            m_tokens.push(tk);
            if(tk.type == TK_EOF)
            {
                break;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

TokenStack& Tokenizer::get_tokens()
{
    return m_tokens;
}

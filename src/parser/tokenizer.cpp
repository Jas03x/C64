#include <tokenizer.hpp>

#include <string>
#include <vector>

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
    bool ret = (pop() == '"');

    const char* ptr = current_ptr();
    unsigned int len = 0;

    if(true)
    {
        while(true)
        {
            char c = pop();
            if(c == 0)
            {
                printf("Error: Unexpected EOF\n");
                ret = false;
                break;
            }
            else if(c == '"')
            {
                break;
            }
            else
            {
                len ++;
            }
        }
    }

    if(ret)
    {
        tk.type = TK_LITERAL;
        tk.literal.type = LITERAL_STRING;
        tk.literal.string.ptr = strdup(ptr);
        tk.literal.string.len = len;

        printf("%.*s => %hhu\n", tk.literal.string.len, tk.literal.string.ptr, tk.type);
    }

    return ret;
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
                case 'F':
                {
                    if(_strncmp(str, "F32", 3))      { tk = { TK_TYPE, { TK_TYPE_F32 } }; }
                    else if(_strncmp(str, "F64", 3)) { tk = { TK_TYPE, { TK_TYPE_F64 } }; }
                    break;
                }
                case 'I':
                {
                    if(_strncmp(str, "I16", 3))      { tk = { TK_TYPE, { TK_TYPE_I16 } }; }
                    else if(_strncmp(str, "I32", 3)) { tk = { TK_TYPE, { TK_TYPE_I32 } }; }
                    else if(_strncmp(str, "I64", 3)) { tk = { TK_TYPE, { TK_TYPE_I64 } }; }
                    break;
                }
                case 'U':
                {
                    if(_strncmp(str, "U16", 3))      { tk = { TK_TYPE, { TK_TYPE_U16 } }; }
                    else if(_strncmp(str, "U32", 3)) { tk = { TK_TYPE, { TK_TYPE_U32 } }; }
                    else if(_strncmp(str, "U64", 3)) { tk = { TK_TYPE, { TK_TYPE_U64 } }; }
                    break;
                }
            }
            
            break;
        }

        case 4:
        {
            if(_strncmp(str, "VOID", 4)) { tk = { TK_TYPE, { TK_TYPE_VOID } }; }
            break;
        }

        case 5:
        {
            if(_strncmp(str, "const", 5)) { tk.type = TK_CONST; break; }
            break;
        }

        case 6:
        {
            if(_strncmp(str, "return", 6))      { tk.type = TK_RETURN; break; }
            else if(_strncmp(str, "extern", 6)) { tk.type = TK_EXTERN; break; }
            else if(_strncmp(str, "struct", 6)) { tk.type = TK_STRUCT; break; }
            break;
        }

        case 9:
        {
            if(_strncmp(str, "namespace", 9) == 0) { tk.type = TK_NAMESPACE; break; }
            break;
        }
    }

    if(tk.type == TK_INVALID)
    {
        tk.type = TK_IDENTIFIER;
        tk.identifier.string.ptr = strdup(str);
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
            case '+': case '-': case '*':
            case '=': case '<': case '>': case ';':
            case '(': case ')': case '[': case ']':
            case '{': case '}': case ',': case '.':
            {
                pop();
                status = process(c, tk) ? STATUS_SUCCESS : STATUS_ERROR;
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

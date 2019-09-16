#include <token_stack.hpp>

#include <debug.hpp>

TokenStack::TokenStack()
{
    m_index = 0;
}

void TokenStack::push(const Token& tk)
{
    m_stack.push_back(tk);
}

Token TokenStack::pop()
{
    Token tk = { TK_INVALID };
    if(m_index < m_stack.size())
    {
        tk = m_stack[m_index++];
    }

    printf("POP: %s", token_to_str(tk));
    switch(tk.type)
    {
        case TK_IDENTIFIER:
        {
            printf(" - %.*s\n", tk.identifier.string.len, tk.identifier.string.ptr);
            break;
        }
        case TK_LITERAL:
        {
            switch(tk.literal.type)
            {
                case LITERAL_INTEGER: { printf(" - integer = %llu\n", tk.literal.integer.value); break; }
                case LITERAL_DECIMAL: { printf(" - decimal = %f\n",   tk.literal.decimal.value); break; }
                case LITERAL_CHAR:    { printf(" - char = %hhu\n",    tk.literal.character);     break; }
                case LITERAL_STRING:  { printf(" - string = \"%.*s\"\n",  tk.literal.string.len, tk.literal.string.ptr); break; }
                default:              { printf(" - invalid\n"); }
            }
            break;
        }
        default: { printf("\n"); break; }
    }

    return tk;
}

Token TokenStack::peek(unsigned int offset)
{
    Token tk = { TK_INVALID };
    if(m_index + offset < m_stack.size())
    {
        tk = m_stack[m_index + offset];
    }
    
    return tk;
}

void TokenStack::clear()
{
    m_index = 0;
    m_stack.clear();
}

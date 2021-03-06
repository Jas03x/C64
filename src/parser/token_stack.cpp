#include <token_stack.hpp>

TokenStack::TokenStack()
{
    m_position = 0;
}

#include <debug.hpp>
Token TokenStack::pop()
{
    Token tk = m_tokens[m_position];
    if (m_position < m_tokens.size() - 1)
    {
        m_position++;
    }
    printf("consume: ");
    print_token(tk);
    return tk;
}

Token TokenStack::peek()
{
    return m_tokens[m_position];
}

Token TokenStack::look_ahead()
{
    Token tk = {};
    if(m_position + 1 < m_tokens.size() - 1)
    {
        tk = m_tokens[m_position];
    }
    return tk;
}

void TokenStack::push(const Token& tk)
{
    m_tokens.push_back(tk);
}

void TokenStack::insert_identifier(const strptr& ptr)
{
    m_identifier_set[ptr] = ptr;
}

const char* TokenStack::find_identifier(const strptr& ptr)
{
    const char* str = nullptr;

    std::map<strptr, strptr>::const_iterator it = m_identifier_set.find(ptr);
    if(it != m_identifier_set.end())
    {
        str = it->second.ptr;
    }

    return str;
}

const std::vector<Token>& TokenStack::get_tokens()
{
    return m_tokens;
}

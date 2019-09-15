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

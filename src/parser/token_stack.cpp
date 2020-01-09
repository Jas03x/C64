#include <token_stack.hpp>

TokenStack::TokenStack()
{
    m_position = 0;
}

const Token& TokenStack::peek()
{
    return m_tokens[m_position];
}

const Token& TokenStack::pop()
{
    return m_tokens[m_position++];
}

void TokenStack::push(const Token& tk)
{
    m_tokens.push_back(tk);
}

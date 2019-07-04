#include <stack.hpp>

Stack::Stack()
{
    m_index = 0;
}

void Stack::push(const Token& tk)
{
    m_stack.push_back(tk);
}

Token Stack::pop()
{
    Token tk = { TK_INVALID };
    if(m_index < m_stack.size())
    {
        tk = m_stack[m_index++];
    }
    return tk;
}

Token Stack::peek(unsigned int offset)
{
    Token tk = { TK_INVALID };
    if(m_index + offset < m_stack.size())
    {
        tk = m_stack[m_index + offset];
    }
    return tk;
}

void Stack::clear()
{
    m_index = 0;
    m_stack.clear();
}

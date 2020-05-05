#include "expression_stack.hpp"

ExpressionStack::ExpressionStack(std::vector<Expression*>* buffer)
{
    m_buffer = buffer;
    m_start = buffer->size();
    m_length = 0;
    m_index = 0;
}

ExpressionStack::~ExpressionStack()
{
    m_buffer->resize(m_buffer->size() - m_length);
}

void ExpressionStack::insert(Expression* expr)
{
    m_buffer->push_back(expr);
    m_length++;
}

bool ExpressionStack::is_empty()
{
    return m_index >= m_length;
}

Expression* ExpressionStack::peek()
{
    return m_index < m_length ? (*m_buffer)[m_start + m_index] : nullptr;
}

Expression* ExpressionStack::pop()
{
    Expression* expr = nullptr;

    if (m_index < m_length)
    {
        expr = (*m_buffer)[m_start + m_index];
        m_index++;
    }

    return expr;
}


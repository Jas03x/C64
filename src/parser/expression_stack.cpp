#include "expression_stack.hpp"

ExpressionStack::ExpressionStack()
{
}

void ExpressionStack::insert(Expression* expr)
{
    m_expr_list.push_back(expr);
}

void ExpressionStack::clear()
{
    m_expr_list.clear();
    m_index = 0;
}

bool ExpressionStack::is_empty()
{
    return m_index < m_expr_list.size();
}

Expression* ExpressionStack::peek()
{
    return m_expr_list[m_index];
}

Expression* ExpressionStack::pop()
{
    return m_expr_list[m_index++];
}


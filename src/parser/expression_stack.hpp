#ifndef EXPRESSION_STACK_HPP
#define EXPRESSION_STACK_HPP

#include <vector>

#include <ast.hpp>

class ExpressionStack
{
private:
    std::vector<Expression*> m_expr_list;
    unsigned int m_index;

public:
    ExpressionStack();

    void insert(Expression* expr);
    void clear();

    bool is_empty();

    Expression* peek();
    Expression* pop();
};

#endif // EXPRESSION_STACK_HPP

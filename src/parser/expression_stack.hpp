#ifndef EXPRESSION_STACK_HPP
#define EXPRESSION_STACK_HPP

#include <vector>

#include <ast.hpp>

class ExpressionStack
{
private:
    std::vector<Expression*>* m_buffer;
    unsigned int m_start;
    unsigned int m_length;
    unsigned int m_index;

public:
    ExpressionStack(std::vector<Expression*>* buffer);
    ~ExpressionStack();

    void insert(Expression* expr);

    bool is_empty();

    Expression* peek();
    Expression* pop();
};

#endif // EXPRESSION_STACK_HPP

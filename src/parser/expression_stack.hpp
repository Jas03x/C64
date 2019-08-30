#ifndef EXPRESSION_STACK_HPP
#define EXPRESSION_STACK_HPP

#include <ast.hpp>
#include <expression_list.hpp>

class ExpressionStack
{
private:
	ExpressionList* m_list;

	ExpressionList::Entry* m_head;
	ExpressionList::Entry* m_tail;

public:
	ExpressionStack(ExpressionList* list);
	~ExpressionStack();

	void push(Expression* expr);
	Expression* pop();
	Expression* peek();
};

#endif // EXPRESSION_STACK_HPP
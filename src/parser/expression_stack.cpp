#include <expression_stack.hpp>

ExpressionStack::ExpressionStack(ExpressionList* list)
{
	m_list = list;

	m_head = nullptr;
	m_tail = nullptr;
}

ExpressionStack::~ExpressionStack()
{
	ExpressionList::Entry* it = m_head;
	while (it != nullptr)
	{
		ExpressionList::Entry* entry = it;
		it = it->next;

		m_list->ret_entry(entry);
	}
}

void ExpressionStack::push(Expression* expr)
{
	ExpressionList::Entry* entry = m_list->get_entry();

	if (entry != nullptr)
	{
		entry->expr = expr;
		
		if (m_head == nullptr)
		{
			m_head = entry;
			m_tail = entry;
		}
		else
		{
			m_tail->next = entry;
			entry->prev = m_tail;
			m_tail = entry;
		}
	}
}

Expression* ExpressionStack::pop()
{
	Expression* expr = nullptr;

	if (m_head != nullptr)
	{
		ExpressionList::Entry* entry = m_head;
		m_head = m_head->next;

		expr = entry->expr;
		m_list->ret_entry(entry);
	}

	return expr;
}

Expression* ExpressionStack::peek()
{
	Expression* expr = nullptr;

	if (m_head != nullptr)
	{
		expr = m_head->expr;
	}

	return expr;
}

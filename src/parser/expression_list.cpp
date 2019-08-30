#include <expression_list.hpp>

ExpressionList::ExpressionList()
{
	m_entries = new Entry[NUM_ENTRIES];

	clear();
}

void ExpressionList::clear()
{
	const unsigned int L = NUM_ENTRIES - 1;

	for (unsigned int i = 0; i < NUM_ENTRIES; i++)
	{
		m_entries[i].prev = (i > 0) ? &m_entries[i - 1] : nullptr;
		m_entries[i].next = (i < L) ? &m_entries[i + 1] : nullptr;
		m_entries[i].expr = nullptr;
	}

	m_list_head = &m_entries[0];
	m_list_tail = &m_entries[L];
}

ExpressionList::~ExpressionList()
{
	delete[] m_entries;
}

ExpressionList::Entry* ExpressionList::get_entry()
{
	Entry* entry = nullptr;

	if (m_list_head != nullptr)
	{
		entry = m_list_head;
		m_list_head = m_list_head->next;

		entry->next = nullptr;
		entry->prev = nullptr;
		entry->expr = nullptr;
	}

	return entry;
}

void ExpressionList::ret_entry(Entry* entry)
{
	m_list_tail->next = entry;
	entry->prev = m_list_tail;
	entry->next = nullptr;
	entry->expr = nullptr;
	m_list_tail = entry;
}
#include <expression_list.hpp>

ExpressionList::ExpressionList()
{
    m_entries = new Entry[NUM_ENTRIES];

    clear();
}

void ExpressionList::clear()
{
    const unsigned int L = NUM_ENTRIES - 1;

    for(unsigned int i = 0; i < NUM_ENTRIES; i++)
    {
        m_entries[i].prev = (i > 0) ? &m_entries[i - 1] : nullptr;
        m_entries[i].next = (i < L) ? &m_entries[i + 1] : nullptr;
        m_entries[i].expr = nullptr;
    }

    m_free_list_head = &m_entries[0];
    m_free_list_tail = &m_entries[L];

    m_used_list_head = nullptr;
    m_used_list_tail = nullptr;
}

ExpressionList::~ExpressionList()
{
    delete[] m_entries;
}

ExpressionList::Entry* ExpressionList::get_entry()
{
    Entry* entry = m_free_list_head;
    m_free_list_head = m_free_list_head->next;
    
    if(m_used_list_head == nullptr)
    {
        m_used_list_head = entry;
        m_used_list_tail = entry;
    }
    else
    {
        m_used_list_tail->next = entry;
        m_used_list_tail = entry;
    }

    return entry;
}

void ExpressionList::ret_entry(Entry* entry)
{
    m_free_list_tail->next = entry;
    entry->next = nullptr;
    entry->prev = m_free_list_tail;
    m_free_list_tail = entry;
}

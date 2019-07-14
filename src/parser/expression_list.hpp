#ifndef EXPRESSION_LIST_HPP
#define EXPRESSION_LIST_HPP

#include <ast.hpp>

class ExpressionList
{
public:
    enum { NUM_ENTRIES = 1024 };

    struct Entry
    {
        Entry*      next;
        Entry*      prev;
        Expression* expr;
    };

private:
    Entry* m_entries;

    Entry* m_free_list_head;
    Entry* m_used_list_head;

    Entry* m_free_list_tail;
    Entry* m_used_list_tail;

public:
    ExpressionList();
    ~ExpressionList();

    Entry* get_entry();
    void   ret_entry(Entry* entry);

    void clear();
};

#endif // EXPRESSION_LIST_HPP
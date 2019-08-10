#ifndef SYMBOL_MAP_H
#define SYMBOL_MAP_H

#include <strptr.hpp>

#include <symbol.hpp>

class SymbolMap
{
private:
    struct Entry
    {
        strptr  name;
        Symbol* symbol;

        Entry*  left;
        Entry*  right;
    };

    enum { MAX_ENTRIES = 1024 };

    Entry*       m_stack[MAX_ENTRIES];
    unsigned int m_index;

private:
    bool    insert_entry(Entry* entry, Entry* symbol);
    Symbol* search_entry(Entry* entry, const strptr& name);
    void    delete_entry(Entry* entry);

public:
    SymbolMap();
    ~SymbolMap();

    bool    insert(const strptr& name, Symbol* symbol);
    Symbol* search(const strptr& name);

    void push_level();
    void pop_level();
};

#endif // SYMBOL_MAP_H
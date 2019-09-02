#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdint.h>

#include <strptr.hpp>

class SymbolTable
{
public:
    struct Entry
    {
        enum
        {
            TYPE_INVALID    = 0,
            TYPE_NAMESPACE  = 1,
            TYPE_SCOPE      = 2,
            TYPE_TYPEDEF    = 3,
            TYPE_COMPOSITE  = 4,
            TYPE_VARIABLE   = 5,
            TYPE_ENUMERATOR = 6,
            TYPE_FUNCTION   = 7
        } type;

        strptr name;
        void*  value;

        Entry* parent;
        
        Entry* left;
        Entry* right;

        Entry();
        ~Entry();

        bool   insert(Entry* entry);
        Entry* search(const strptr& sym_name);
    };

    Entry* m_global;
    Entry* m_current;

public:
    SymbolTable();
    ~SymbolTable();

    bool pop_scope();
    bool push_scope(Entry* entry);

    Entry* current_scope();
};

#endif // SYMBOL_TABLE_H
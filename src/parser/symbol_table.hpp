#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <stdint.h>

#include <strptr.hpp>

enum SYM_TYPE
{
    SYM_INVALID    = 0x0,
    SYM_VARIABLE   = 0x1,
    SYM_FUNCTION   = 0x2,
    SYM_COMPOSITE  = 0x3,
    SYM_ENUMERATOR = 0x4,
    SYM_TYPE_MAX   = 0x5
};

class SymbolTable
{
public:
    struct Entry
    {
        uint8_t type;
        strptr  name;

        Entry* l_child;
        Entry* r_child;
    };

    struct Scope
    {
        Entry* root;
        Scope* next;
        Scope* prev;
    };

private:
    const Entry* search(const strptr& name, const Entry* entry);

public:
    SymbolTable();
    ~SymbolTable();

    void pop();
    void push();

    bool insert(Entry* entry);
    const Entry* search(const strptr& name);
};

#endif
#include <symbol_map.hpp>

#include <string.h>

int _strncmp(const char* str0, const char* str1, unsigned int len)
{
    int ret = 0;

    for(unsigned int i = 0; i < len; i++)
    {
        if(str0[i] != str1[i])
        {
            ret = str0[i] < str1[i] ? -1 : 1;
            break;
        }
    }

    return ret;
}

Symbol* SymbolMap::search_entry(SymbolMap::Entry* entry, const strptr& name)
{
    Symbol* sym = nullptr;

    if(entry != nullptr)
    {
        if(entry->name.len > name.len)
        {
            sym = search_entry(entry->right, name);
        }
        else if(entry->name.len < name.len)
        {
            sym = search_entry(entry->left, name);
        }
        else
        {
            int dif = _strncmp(name.ptr, entry->name.ptr, name.len);

            if(dif == 0)
            {
                sym = entry->symbol;
            }
            else if(dif < 0)
            {
                sym = search_entry(entry->left, name);
            }
            else
            {
                sym = search_entry(entry->right, name);
            }
        }
    }

    return sym;
}

bool SymbolMap::insert_entry(SymbolMap::Entry* entry, SymbolMap::Entry* symbol)
{
    bool ret = false;

    if(entry != nullptr)
    {
        if(entry->name.len > symbol->name.len)
        {
            if(entry->right == nullptr) { entry->right = symbol; ret = true;        }
            else                        { ret = insert_entry(entry->right, symbol); }
        }
        else if(entry->name.len < symbol->name.len)
        {
            if(entry->left == nullptr) { entry->left = symbol; ret = true;        }
            else                       { ret = insert_entry(entry->left, symbol); }
        }
        else
        {
            int dif = _strncmp(symbol->name.ptr, entry->name.ptr, symbol->name.len);

            if(dif == 0)
            {
                ret = false;
            }
            else if(dif < 0)
            {
                if(entry->left == nullptr) { entry->left = symbol; ret = true;        }
            else                           { ret = insert_entry(entry->left, symbol); }
            }
            else
            {
                if(entry->right == nullptr) { entry->right = symbol; ret = true; }
                else                        { ret = insert_entry(entry->right, symbol); }
            }
        }
    }

    return ret;
}

void SymbolMap::delete_entry(SymbolMap::Entry* entry)
{
    if(entry->left  != nullptr) delete entry->left;
    if(entry->right != nullptr) delete entry->right;
    delete entry;
}

SymbolMap::SymbolMap()
{
    m_index = 0;
    memset(m_stack, 0, sizeof(m_stack));
}

SymbolMap::~SymbolMap()
{
    for(int i = m_index; i >= 0; i--)
    {
        delete_entry(m_stack[i]);
        m_stack[i] = nullptr;
    }
}

bool SymbolMap::insert(const strptr& name, Symbol* symbol)
{
    bool status = false;

    SymbolMap::Entry* entry = new SymbolMap::Entry();
    entry->name = name;
    entry->symbol = symbol;

    if(m_stack[m_index] == nullptr)
    {
        m_stack[m_index] = entry;
        status = true;
    }
    else
    {
        status = insert_entry(m_stack[m_index], entry);
    }

    return status;
}

Symbol* SymbolMap::search(const strptr& name)
{
    Symbol* sym = nullptr;

    for(int i = m_index; i >= 0; i--)
    {
        sym = search_entry(m_stack[i], name);

        if(sym != nullptr) { break; }
    }

    return sym;
}

void SymbolMap::push_level()
{
    m_index ++;
}

void SymbolMap::pop_level()
{
    if(m_stack[m_index] != nullptr)
    {
        delete_entry(m_stack[m_index]);
        m_stack[m_index] = nullptr;
    }
}

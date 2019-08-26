#include <symbol_table.hpp>

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

SymbolTable::Entry::Entry()
{
    name = {};
    type = SymbolTable::Entry::TYPE_INVALID;
    
    value  = nullptr;
    child  = nullptr;
    parent = nullptr;
    left   = nullptr;
    right  = nullptr;
}

SymbolTable::Entry::~Entry()
{
    if(value != nullptr) delete value;
    if(child != nullptr) delete child;
    if(left  != nullptr) delete left;
    if(right != nullptr) delete right;
}

SymbolTable::Entry* SymbolTable::Entry::search(const strptr& sym_name)
{
    SymbolTable::Entry* entry = nullptr;

    if(this->name.len > sym_name.len)
    {
        if(this->right != nullptr) this->right->search(sym_name);
    }
    else if(this->name.len < sym_name.len)
    {
        if(this->left != nullptr) this->left->search(sym_name);
    }
    else
    {
        int dif = _strncmp(sym_name.ptr, this->name.ptr, sym_name.len);

        if(dif == 0)
        {
            entry = this;
        }
        else if(dif < 0)
        {
            if(this->left != nullptr) this->left->search(sym_name);
        }
        else
        {
            if(this->right != nullptr) this->right->search(sym_name);
        }
    }

    return entry;
}

bool SymbolTable::Entry::insert(SymbolTable::Entry* entry)
{
    bool ret = false;

    if(this->name.len > entry->name.len)
    {
        if(this->right == nullptr) { this->right = entry; ret = true;  }
        else                       { ret = this->right->insert(entry); }
    }
    else if(this->name.len < entry->name.len)
    {
        if(this->left == nullptr) { this->left = entry; ret = true;  }
        else                      { ret = this->left->insert(entry); }
    }
    else
    {
        int dif = _strncmp(entry->name.ptr, this->name.ptr, entry->name.len);

        if(dif == 0)
        {
            ret = false;
        }
        else if(dif < 0)
        {
            if(this->left == nullptr) { this->left = entry; ret = true;  }
            else                      { ret = this->left->insert(entry); }
        }
        else
        {
            if(this->right == nullptr) { this->right = entry; ret = true;  }
            else                       { ret = this->right->insert(entry); }
        }
    }

    return ret;
}

SymbolTable::SymbolTable()
{
    m_global = new SymbolTable::Entry();
    m_global->type = SymbolTable::Entry::TYPE_NAMESPACE;
}

SymbolTable::~SymbolTable()
{
    delete m_global;
    
    m_global  = nullptr;
    m_current = nullptr;
}

bool SymbolTable::push_scope(SymbolTable::Entry* entry)
{
    bool status = m_current->insert(entry);
    
    if(status)
    {
        m_current = entry;
    }

    return status;
}

bool SymbolTable::pop_scope()
{
    bool status = m_current->parent != nullptr;

    if(status)
    {
        SymbolTable::Entry* entry = m_current;
        m_current = m_current->parent;

        switch(entry->type)
        {
            case SymbolTable::Entry::TYPE_SCOPE:
            case SymbolTable::Entry::TYPE_FUNCTION:
            {
                delete entry;
            }
        }
    }
}

SymbolTable::Entry* SymbolTable::current_scope()
{
    return m_current;
}

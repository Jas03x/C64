#include <symbol_table.hpp>

#include <stdio.h>
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
    
    parent = nullptr;
    left   = nullptr;
    right  = nullptr;
	children = nullptr;
}

SymbolTable::Entry::~Entry()
{
    if(left      != nullptr) delete left;
    if(right     != nullptr) delete right;
	if (children != nullptr) delete children;
}

SymbolTable::Entry* SymbolTable::Entry::search(const strptr& sym_name)
{
	printf("search for %.*s\n", sym_name.len, sym_name.ptr);
	SymbolTable::Entry* entry = nullptr;
	SymbolTable::Entry* it = this;

	while(it != nullptr)
	{
		entry = it->children;

		while (entry != nullptr)
		{
			if      (entry->name.len > sym_name.len) { entry = entry->right; }
			else if (entry->name.len < sym_name.len) { entry = entry->left;  }
			else
			{
				int dif = _strncmp(sym_name.ptr, entry->name.ptr, sym_name.len);
			
				if (dif == 0)     { break;                }
				else if (dif < 0) { entry = entry->left;  }
				else              { entry = entry->right; }
			}
		}

		if (entry != nullptr) { break;           }
		else                  { it = it->parent; }
	}

    return entry;
}

bool SymbolTable::Entry::insert(SymbolTable::Entry* entry)
{
	printf("insert %.*s\n", entry->name.len, entry->name.ptr);
	bool status = true;
	SymbolTable::Entry** target = nullptr;

	if (children == nullptr)
	{
		target = &children;
	}
	else
	{
		Entry* it = this->children;
		while (status && (it != nullptr) && (target == nullptr))
		{
			if (it->name.len > entry->name.len)
			{
				if (it->right == nullptr) { target = &it->right; }
				else                      { it = it->right;      }
			}
			else if (this->name.len < entry->name.len)
			{
				if (it->left == nullptr) { target = &it->left; }
				else                     { it = it->left;      }
			}
			else
			{
				int dif = _strncmp(entry->name.ptr, it->name.ptr, entry->name.len);
				if (dif == 0)
				{
					status = false;
					printf("error: duplicated definition of identifier \"%.*s\"\n", entry->name.len, entry->name.ptr);
				}
				else if (dif < 0)
				{
					if (it->left == nullptr) { target = &it->left; }
				    else                     { it = it->left;      }
				}
				else
				{
					if (it->right == nullptr) { target = &it->right; }
					else                      { it = it->right;      }
				}
			}
		}
	}

	if(status)
	{
		if (target == nullptr) { status = false;  }
		else                   { *target = entry; }
	}

    return status;
}

SymbolTable::SymbolTable()
{
    m_global = new SymbolTable::Entry();
    m_global->type = SymbolTable::Entry::TYPE_NAMESPACE;

	m_current = m_global;
}

SymbolTable::~SymbolTable()
{
    delete m_global;
    
    m_global  = nullptr;
    m_current = nullptr;
}

bool SymbolTable::push_scope(SymbolTable::Entry* entry)
{
	bool status = true;
	
	if (entry->name.len > 0) {
		status = m_current->insert(entry);
	}

	if (status)
	{
		entry->parent = m_current;
		m_current = entry;
	}

	return status;
}

bool SymbolTable::pop_scope()
{
    bool status = m_current->parent != nullptr;

    if(status)
    {
        m_current = m_current->parent;
    }

	return status;
}

SymbolTable::Entry* SymbolTable::current_scope()
{
    return m_current;
}

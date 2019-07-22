#include <scope.hpp>

#include <stdio.h>
#include <string.h>

Scope::Scope()
{
    m_index = 0;
    memset(m_stack, 0, sizeof(m_stack));
}

bool Scope::is_global()
{
    return (m_index == 0);
}

bool Scope::insert_function(Statement* stmt)
{
    bool status = true;

    if(!is_global())
    {
        status = false;
    }
    else
    {
        m_stack[++m_index] = stmt;
    }

    return status;
}

bool Scope::append(Statement* stmt)
{
    bool status = true;

    if(m_stack[m_index] == nullptr)
    {
        status = false;
    }
    else
    {
        m_stack[m_index]->next = stmt;
        m_stack[m_index] = stmt;
    }

    return true;
}

bool Scope::push(Statement* stmt)
{
    m_stack[++m_index] = stmt;

    return true;
}

bool Scope::pop()
{
    bool status = true;

    if(m_stack[m_index] != nullptr)
    {
        m_stack[m_index] = nullptr;

        if(m_index > 0)
        {
            m_index --;
        }
    }
    else
    {
        status = false;
    }
    
    return status;
}

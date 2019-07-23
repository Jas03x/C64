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

bool Scope::append(Statement* stmt)
{
    if(m_stack[m_index] != nullptr)
    {
        m_stack[m_index]->next = stmt;
    }

    m_stack[m_index] = stmt;
    return true;
}

bool Scope::push()
{
    bool status = false;

    if(m_index < MAX_STMT_DEPTH)
    {
        m_index ++;
        status = true;
    }

    return status;
}

bool Scope::pop()
{
    bool status = false;

    if((m_index > 0) && (m_stack[m_index] != nullptr))
    {
        m_stack[m_index] = nullptr;
        m_index --;
        
        status = true;
    }

    return status;
}

#ifndef SCOPE_HPP
#define SCOPE_HPP

#include <ast.hpp>

class Scope
{
private:
    enum
    {
        MAX_STMT_DEPTH = 128
    };

private:
    Statement*   m_stack[MAX_STMT_DEPTH];
    unsigned int m_index;

public:
    Scope();

    bool is_global();
    
    bool push();
    bool pop();

    bool append(Statement* stmt);
};

#endif // SCOPE_HPP
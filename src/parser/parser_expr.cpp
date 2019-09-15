#include <parser.hpp>

bool Parser::parse_expression(Expression** ptr)
{
    bool status = true;

    while(m_stack->peek(0).type != TK_SEMICOLON) { m_stack->pop(); }
    
    *ptr = new Expression();
    (*ptr)->type = EXPR_INVALID;

    return status;
}

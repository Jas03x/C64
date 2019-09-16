#include <parser.hpp>

bool Parser::parse_expression(Expression** ptr)
{
    bool status = true, running = true;

    while(status && running)
    {
        Token tk = m_stack->peek(0);
        switch(tk.type)
        {
            // case TK_COLON:
            case TK_COMMA:
            case TK_SEMICOLON:
            case TK_CLOSE_CURLY_BRACKET:
            {
                running = false;
                break;
            }

            default:
            {
                m_stack->pop();
                break;
            }
        }
    }
    
    if(status)
    {
        *ptr = new Expression();
        (*ptr)->type = EXPR_INVALID;
    }

    return status;
}
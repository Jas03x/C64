#include <parser.hpp>

Parser::Parser(TokenStack* stack)
{
    m_stack = stack;
}

AST* Parser::Parse(TokenStack* stack)
{
    bool status = true;
    
    Parser parser(stack);
    AST* ast = new AST();

    while(status)
    {
        if(stack->peek(0).type == TK_EOF) { break; }
        else
        {
            if(!parser.parse_statement(ast->statements))
            {
                status = false;
            }
        }
    }

    if(!status)
    {
        delete ast;
        ast = nullptr;
    }

    return ast;
}

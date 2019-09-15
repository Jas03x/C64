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
            Statement* stmt = nullptr;
            if(parser.parse_statement(&stmt))
            {
                if(stmt != nullptr) {
                    ast->statements.insert(stmt);
                }
            } else {
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

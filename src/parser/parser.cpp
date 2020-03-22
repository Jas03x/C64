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
    }

    if(!status)
    {
        delete_ast(ast);
        ast = nullptr;
    }

    return ast;
}

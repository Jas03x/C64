#include <parser.hpp>

Parser::Parser(TokenStack& stack)
{
    m_stack = &stack;
}

AST* Parser::Parse(TokenStack& stack)
{
    bool status = true;

    Parser parser(stack);
    AST* ast = new AST();
    
    while(status)
    {
        Statement* stmt = new Statement();
        ast->statements.insert(stmt);

        status = parse_statement(stmt);
    }

    if(!status)
    {
        delete_ast(ast);
        ast = nullptr;
    }

    return ast;
}

bool parse_statement(Statement* stmt)
{
    return true;
}

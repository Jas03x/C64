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
        Statement* stmt = new Statement();
        ast->statements.insert(stmt);

        status = parser.parse_statement(*stmt);
    }

    if(!status)
    {
        delete_ast(ast);
        ast = nullptr;
    }

    return ast;
}

bool Parser::parse_statement(Statement& stmt)
{
    bool status = true;
    switch(m_stack->peek(0).type)
    {
        case TK_TYPE:
        {
            status = parse_declaration(stmt);
            break;
        }
    }
    return status;
}

bool Parser::parse_declaration(Statement& stmt)
{
    bool status = true;
    switch(m_stack->peek(0).type)
    {
        case TK_TYPE:
        {
            status = 
            break;
        }
    }
    return status;
}

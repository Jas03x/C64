#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

#include <ast.hpp>
#include <token_stack.hpp>

class Parser
{
private:
	TokenStack* m_stack;

private:
    Parser(TokenStack* stack);

    bool parse_statement(Statement& stmt);
    bool parse_declaration(Statement& stmt);

public:
    static AST* Parse(TokenStack* stack);
};

#endif // PARSER_HPP

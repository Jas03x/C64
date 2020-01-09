#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

#include <ast.hpp>
#include <symbol_table.hpp>
#include <token_stack.hpp>

class Parser
{
private:
	TokenStack* m_stack;

private:
    Parser(TokenStack* stack);

public:
    static AST* Parse(TokenStack* stack);
};

#endif // PARSER_HPP

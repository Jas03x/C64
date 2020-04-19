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
    Parser(TokenStack& stack);

    bool accept(uint8_t type);
    bool expect(uint8_t type); 

    bool parse_type(Type** ptr);
    bool parse_identifier(strptr* id);
    bool parse_definition(Statement* stmt);
    bool parse_global_statement(Statement* stmt);

public:
    static AST* Parse(TokenStack& stack);
};

#endif // PARSER_HPP

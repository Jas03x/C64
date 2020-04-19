#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

#include <ast.hpp>
#include <token_stack.hpp>

class Parser
{
private:
	TokenStack* m_stack;

    std::vector<Expression*> m_expr_stack;

private:
    Parser(TokenStack& stack);

    bool accept(uint8_t type);
    bool expect(uint8_t type); 

    bool parse_type(Type** ptr);
    bool parse_identifier(strptr* id);
    bool parse_definition(Statement* stmt);
    bool parse_statement(Statement** ptr);
    bool parse_parameter(Parameter** ptr);
    bool parse_global_statement(Statement* stmt);
    bool parse_function_body(List<Statement>* body);
    bool parse_return_statement(Statement** ptr);
    bool parse_expression(Expression** expr);
    bool parse_expr_stmt(Statement** ptr);
    bool parse_function_definition(Type* ret_type, strptr name, Statement* stmt);

public:
    static AST* Parse(TokenStack& stack);
};

#endif // PARSER_HPP

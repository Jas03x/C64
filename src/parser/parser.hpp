#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

#include <ast.hpp>
#include <token_stack.hpp>
#include <expression_stack.hpp>

class Parser
{
private:
    bool m_status;
	TokenStack* m_stack;

    std::vector<Expression*> m_expr_stack;

private:
    Parser(TokenStack& stack);
    
    void error(const char* msg);
    void unexpected_token(uint8_t tk, uint8_t ex);

    bool accept(uint8_t type);
    bool expect(uint8_t type);

    unsigned int get_op_precedence(uint8_t op);
    Expression* process_expr_operand(ExpressionStack* stack);
    Expression* process_expression(ExpressionStack* stack, Expression* lhs, uint8_t min);

    bool parse_type(Type** ptr);
    bool parse_identifier(strptr* id);
    bool parse_definition(Statement* stmt);
    bool parse_statement(Statement** ptr);
    bool parse_parameter(Function::Parameter** ptr);
    bool parse_global_statement(Statement* stmt);
    bool parse_function_body(List<Statement>* body);
    bool parse_return_statement(Statement** ptr);
    bool parse_expression(Expression** ptr);
    bool parse_expression(Statement** ptr);
    bool parse_function_definition(Type* ret_type, strptr name, Statement* stmt);
    bool parse_expr_literal(Expression** ptr);
    bool parse_expr_identifier(Expression** ptr);
    bool parse_expr_operator(Expression** ptr);
    bool parse_expr_args(Expression** ptr);

public:
    static AST* Parse(TokenStack& stack);
};

#endif // PARSER_HPP

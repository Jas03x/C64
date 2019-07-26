#ifndef PARSER_HPP
#define PARSER_HPP

#include <token_stack.hpp>
#include <expression_list.hpp>

class Parser
{
private:
    enum
    {
        PRECEDENCE_LEVEL_INVALID = 0,
        PRECEDENCE_LEVEL_1       = 1,
        PRECEDENCE_LEVEL_2       = 2,
        PRECEDENCE_LEVEL_3       = 3,
        PRECEDENCE_LEVEL_4       = 4,
        PRECEDENCE_LEVEL_5       = 5,
        PRECEDENCE_LEVEL_6       = 6,
        PRECEDENCE_LEVEL_7       = 7,
        PRECEDENCE_LEVEL_8       = 8,
        PRECEDENCE_LEVEL_MAX     = 9
    };

private:
    TokenStack*      m_stack;
    ExpressionList   m_list;

private:
    Parser(TokenStack* stack);
    ~Parser();

    bool check_operator_precedence(unsigned int precedence_level, uint8_t op);

    AST* parse();

    bool process_expression(ExpressionList::Entry* list, Expression** expr);

    bool parse_statement(Statement** ptr);
    bool parse_value(Expression** ptr);
    bool parse_operator(Expression** ptr);
    bool parse_expression(Expression** ptr);

    bool parse_modifiers(VariableModifiers& var_mod, FunctionModifiers& func_mod);
    bool parse_arguments(Argument** args);
    bool parse_parameters(Parameter** params);

    bool parse_body(Statement** stmt);
    bool parse_type(DataType& type);
    bool parse_if_stmt(Statement** stmt);
    bool parse_return(Statement** stmt);
    bool parse_declaration(Statement** stmt);
    bool parse_struct_declaration(Statement** ptr);

public:
    static AST* Parse(TokenStack* stack);

};

#endif // PARSER_HPP

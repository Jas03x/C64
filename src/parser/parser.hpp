#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

#include <symbol.hpp>
#include <symbol_map.hpp>
#include <token_stack.hpp>
#include <expression_list.hpp>

class Parser
{
private:
    enum
    {
        PRECEDENCE_LEVEL_INVALID =  0,
        PRECEDENCE_LEVEL_1       =  1,
        PRECEDENCE_LEVEL_2       =  2,
        PRECEDENCE_LEVEL_3       =  3,
        PRECEDENCE_LEVEL_4       =  4,
        PRECEDENCE_LEVEL_5       =  5,
        PRECEDENCE_LEVEL_6       =  6,
        PRECEDENCE_LEVEL_7       =  7,
        PRECEDENCE_LEVEL_8       =  8,
        PRECEDENCE_LEVEL_9       =  9,
        PRECEDENCE_LEVEL_MAX     = 10
    };

private:
    TokenStack*    m_stack;
    ExpressionList m_list;

private:
    Parser(TokenStack* stack);
    ~Parser();

    bool check_operator_precedence(unsigned int precedence_level, uint8_t op);

    bool parse(AST** ptr);

    bool process_expression(ExpressionList::Entry* list, Expression** expr);

    bool parse_statement(Statement** ptr);
    bool parse_value(Expression** ptr);
    bool parse_operator(Expression** ptr);
    bool parse_expression(Expression** ptr);

    bool parse_modifiers(VariableFlags& flags);
    bool parse_arguments(Argument** args);
    bool parse_parameters(Parameter** params);
    bool parse_array(Variable** variable);
    bool parse_initializer(Expression** ptr);

    bool parse_enum(Enum** ptr);

    bool parse_enum_definition(Statement** ptr);
    bool parse_switch_stmt(Statement** ptr);
    bool parse_default_stmt(Statement** ptr);
    bool parse_case_stmt(Statement** ptr);
    bool parse_continue_stmt(Statement** ptr);
    bool parse_label(Statement** ptr);
    bool parse_goto_stmt(Statement** ptr);
    bool parse_break_stmt(Statement** ptr);
    bool parse_compound_stmt(Statement** ptr);
    bool parse_typedef(Statement** ptr);
    bool parse_namespace(Statement** ptr);
	bool parse_identifier(Identifier** identifier);
    bool parse_body(Statement** stmt);
    bool parse_for_stmt(Statement** ptr);
    bool parse_while_stmt(Statement** ptr);
    bool parse_variable(Variable** ptr);
    bool parse_if_stmt(Statement** stmt);
    bool parse_else_stmt(Statement** ptr);
    bool parse_return(Statement** stmt);
    bool parse_declaration(Statement** stmt);
    bool parse_struct(Structure** ptr);
    bool parse_struct_definition(Statement** ptr);
    bool parse_function_decl(Variable* var, strptr name, Statement** ptr);
    bool parse_variable_decl(Variable* var, strptr name, Statement** ptr);

public:
    static AST* Parse(TokenStack* stack);
};

#endif // PARSER_HPP

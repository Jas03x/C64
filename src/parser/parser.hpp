#ifndef PARSER_HPP
#define PARSER_HPP

#include <ast.hpp>
#include <token_stack.hpp>
#include <expression_list.hpp>
#include <linked_list.hpp>
#include <scope.hpp>

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

enum
{
    MAX_STMT_DEPTH = 128
};

class Parser
{
private:
    union Arg
    {
        struct
        {
            uint8_t type;
            strptr  name;
        } decl;
    };

private:
    TokenStack*    m_stack;
    ExpressionList m_list;

    LList<Statement> m_global_stmt_list;

    Scope m_scope;

private:
    bool parse_decl();

    bool read_arguments(Argument** args);

    Expression* read_value();
    Expression* read_expression();
    Expression* read_operator();

    Expression* process_expression(ExpressionList::Entry* expression);

    bool check_operator_precedence(unsigned int precedence_level, uint8_t op);

    bool parse_statement();
    bool parse_return();
    bool parse_function(Arg& arg);
    bool parse_variable(Arg& arg);
    bool parse_if_stmt();
    bool parse_expr_stmt();

    bool handle_end_of_block();

    bool insert_function(Statement* stmt);
    bool insert_statement(Statement* stmt);

public:
    Parser(TokenStack* tokens);

    bool process();

    AST* get_ast();
};

#endif // PARSER_HPP

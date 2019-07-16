#ifndef PARSER_HPP
#define PARSER_HPP

#include <ast.hpp>
#include <token_stack.hpp>
#include <expression_list.hpp>

enum
{
    PRECEDENCE_LEVEL_INVALID = 0,
    PRECEDENCE_LEVEL_1       = 1,
    PRECEDENCE_LEVEL_2       = 2,
    PRECEDENCE_LEVEL_MAX     = 3
};

class Parser
{
private:
    Root* m_root;

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

    Function* m_func_ptr;
    Function* m_func_tail;

    Statement* m_stmt_tail;

private:
    bool parse_decl();

    bool read_arguments(Argument** args);

    Expression* read_value();
    Expression* read_expression();

    Expression* process_expression(ExpressionList::Entry* expression);

    bool check_operator_precedence(unsigned int precedence_level, uint8_t op);

    bool parse_statement();
    bool parse_return();
    bool parse_function(Arg& arg);
    bool parse_variable(Arg& arg);

    bool handle_end_of_block();

    bool insert_function(Function* func);
    bool insert_statement(Statement* stmt);

public:
    Parser(TokenStack* tokens);

    bool process();

    Root* get_ast();
};

#endif // PARSER_HPP
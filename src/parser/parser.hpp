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

    Function**  m_func_ptr;
    Statement** m_stmt_ptr;

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

private:
    bool parse_decl();

    Expression* read_value();
    Expression* read_expression();

    Expression* process_expression(ExpressionList::Entry* expression);

    bool check_operator_precedence(unsigned int precedence_level, uint8_t op);

    bool parse_function(Arg& arg);
    bool parse_variable(Arg& arg);

public:
    Parser(TokenStack* tokens);

    bool process();
};

#endif // PARSER_HPP
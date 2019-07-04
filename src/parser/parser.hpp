#ifndef PARSER_HPP
#define PARSER_HPP

#include <literal.hpp>
#include <stack.hpp>
#include <token.hpp>

struct Expression; // forward declare "expression" for "argument"

struct Argument
{
    Expression* value;
    Argument*   next;
};

enum
{
    EXPR_INVALID    = 0x0,
    EXPR_LITERAL    = 0x1,
    EXPR_IDENTIFIER = 0x2,
    EXPR_ASSIGN     = 0x3,
    EXPR_ADD        = 0x4,
    EXPR_SUB        = 0x5,
    EXPR_MUL        = 0x6,
    EXPR_DIV        = 0x7,
    EXPR_CALL       = 0x8
};

struct Expression
{
    uint8_t type;

    union
    {
        Literal literal;

        struct
        {
            strptr name;
        } identifier;

        struct
        {
            Expression* lhs;
            Expression* rhs;
        } assign;

        struct
        {
            Expression* lhs;
            Expression* rhs;
        } add;

        struct
        {
            Expression* lhs;
            Expression* rhs;
        } sub;

        struct
        {
            Expression* lhs;
            Expression* rhs;
        } mul;

        struct
        {
            Expression* lhs;
            Expression* rhs;
        } div;

        struct
        {
            strptr    func_name;
            Argument* arguments;
        } call;
    };
};

enum
{
    STMT_INVALID  = 0x0,
    STMT_EXPR     = 0x1,
    STMT_DECL_VAR = 0x2,
    STMT_IF       = 0x3,
    STMT_RET      = 0x4
};

struct Statement
{
    uint8_t type;

    union
    {
        Expression* expr;

        struct
        {
            uint8_t     type;
            strptr      name;
            Expression* value;
        } decl_var;

        struct
        {
            Expression* condition;
            Statement*  body;
        } if_stmt;

        struct
        {
            Expression* expression;
        } ret_stmt;
    };

    Statement* next;
};

struct Parameter
{
    uint8_t    type;
    strptr     name;
    Parameter* next;
};

struct Function
{
    uint8_t    ret_type;
    strptr     name;
    Parameter* params;
    Statement* body;
    Function*  next;
};

struct Root
{
    Function* functions;
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
    Stack* m_stack;

private:
    bool parse_decl();

    bool parse_function(Arg& arg);
    bool parse_variable(Arg& arg);

    Expression* read_expression();

public:
    Parser(Stack* tokens);

    bool process();
};

#endif // PARSER_HPP
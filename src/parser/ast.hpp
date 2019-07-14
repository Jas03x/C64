#ifndef AST_HPP
#define AST_HPP

#include <literal.hpp>
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
    EXPR_OPERATION  = 0x4,
    EXPR_CALL       = 0x5
};

enum
{
    EXPR_OP_INVALID = 0x0,
    EXPR_OP_ADD     = 0x1,
    EXPR_OP_SUB     = 0x2,
    EXPR_OP_MUL     = 0x3,
    EXPR_OP_DIV     = 0x4
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
            uint8_t     op;
            Expression* lhs;
            Expression* rhs;
        } operation;

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

#endif // AST_HPP
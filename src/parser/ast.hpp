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
    EXPR_OP_INVALID                = 0x00,
    EXPR_OP_ADD                    = 0x01,
    EXPR_OP_SUB                    = 0x02,
    EXPR_OP_MUL                    = 0x03,
    EXPR_OP_DIV                    = 0x04,
    EXPR_OP_LOGICAL_NOT            = 0x05,
    EXPR_OP_LOGICAL_AND            = 0x06,
    EXPR_OP_LOGICAL_OR             = 0x07,
    EXPR_OP_BITWISE_COMPLEMENT     = 0x08,
    EXPR_OP_BITWISE_XOR            = 0x09,
    EXPR_OP_BITWISE_AND            = 0x0A,
    EXPR_OP_BITWISE_OR             = 0x0B,
    EXPR_OP_BITWISE_L_SHIFT        = 0x0C,
    EXPR_OP_BITWISE_R_SHIFT        = 0x0D,
    EXPR_OP_CMP_EQUAL              = 0x0E,
    EXPR_OP_CMP_NOT_EQUAL          = 0x0F,
    EXPR_OP_CMP_LESS_THAN          = 0x10,
    EXPR_OP_CMP_MORE_THAN          = 0x11,
    EXPR_OP_CMP_LESS_THAN_OR_EQUAL = 0x12,
    EXPR_OP_CMP_MORE_THAN_OR_EQUAL = 0x13,
    EXPR_OP_REFERENCE              = 0x14,
    EXPR_OP_DEREFERENCE            = 0x15,
    EXPR_OP_ASSIGN                 = 0x16,
    EXPR_OP_ARROW                  = 0x17
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
    STMT_INVALID       = 0x0,
    STMT_ROOT          = 0x1,
    STMT_FUNCTION_DECL = 0x2,
    STMT_FUNCTION_DEF  = 0x3,
    STMT_BLOCK         = 0x4,
    STMT_EXPR          = 0x5,
    STMT_VARIABLE      = 0x6,
    STMT_IF            = 0x7,
    STMT_RET           = 0x8
};

union VariableModifiers
{
    struct
    {
        unsigned int is_constant         : 1;
        unsigned int is_pointer          : 1;
        unsigned int is_fixed_size_array : 1;
    };

    uint8_t value;
};

union FunctionModifiers
{
    struct
    {
    };

    uint8_t value;
};

struct DataType
{
    uint8_t type;
    
    VariableModifiers flags;

    union
    {
        uint32_t array_size; // used if flags.is_fixed_size_array = 1
    };
};

struct Parameter
{
    DataType   type;
    strptr     name;
    Parameter* next;
};

struct Statement
{
    uint8_t type;

    union
    {
        struct
        {
            // no members
        } block;

        Expression* expr;

        struct
        {
            strptr     name;
            DataType   ret_type;
            Parameter* params;
            Statement* body;

            FunctionModifiers flags;
        } function;

        struct
        {
            strptr      name;
            DataType    type;
            Expression* value;
        } variable;

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

struct AST
{
    Statement* statements;
};

#endif // AST_HPP

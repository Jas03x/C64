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
    EXPR_INVALID     = 0x0,
    EXPR_SUB_EXPR    = 0x1,
    EXPR_LITERAL     = 0x2,
    EXPR_IDENTIFIER  = 0x3,
    EXPR_ASSIGN      = 0x4,
    EXPR_OPERATION   = 0x5,
    EXPR_CALL        = 0x6,
    EXPR_INITIALIZER = 0x7
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
    EXPR_OP_ACCESS_FIELD           = 0x17,
    EXPR_OP_ARROW                  = 0x18,
    EXPR_OP_INDEX                  = 0x19
};

struct Initializer
{
    struct Value
    {
        Expression* expr;
        Value*      next;
    };
    
    Value* values;
};

struct Expression
{
    uint8_t type;

    union
    {
        Literal literal;

        Expression* sub_expr;

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
            Expression* function;
            Argument*   arguments;
        } call;

        Initializer initializer;
    };
};

enum
{
    STMT_INVALID       = 0x0,
    STMT_FUNCTION_DECL = 0x1,
    STMT_FUNCTION_DEF  = 0x2,
    STMT_EXPR          = 0x3,
    STMT_VARIABLE_DECL = 0x4,
    STMT_IF            = 0x5,
    STMT_RET           = 0x6,
    STMT_STRUCT_DEF    = 0x7,
    STMT_FOR           = 0x8,
    STMT_WHILE         = 0x9
};

enum TYPE
{
    TYPE_UNKNOWN = 0x0,
    TYPE_VOID    = 0x1,
    TYPE_U8      = 0x2,
    TYPE_U16     = 0x3,
    TYPE_U32     = 0x4,
    TYPE_U64     = 0x5,
    TYPE_I8      = 0x6,
    TYPE_I16     = 0x7,
    TYPE_I32     = 0x8,
    TYPE_I64     = 0x9,
    TYPE_F32     = 0xA,
    TYPE_F64     = 0xB,
    TYPE_PTR     = 0xC,
    TYPE_STRUCT  = 0xD,
    TYPE_CONSTANT_SIZED_ARRAY = 0xE,
    TYPE_VARIABLE_SIZED_ARRAY = 0xF
};

struct Variable;

struct Structure
{
    strptr name;

    struct Member
    {
        strptr    name;
        Variable* variable;

        Member*   next;
    };

    Member* members;
};

union VariableFlags
{
    struct
    {
        unsigned int is_constant         : 1;
        unsigned int is_external_symbol  : 1;
    };

    uint8_t value;
};

struct Variable
{
    uint8_t type;
    VariableFlags flags;

    union
    {
        strptr identifier;

        struct
        {
            Expression*     size;
            const Variable* elements;
        } array;

        Structure* structure;

        const Variable* pointer;
    };
};

struct Parameter
{
    strptr     name;
    Variable*  type;
    Parameter* next;
};

struct Statement;

struct Function
{
    Variable*  ret_type;
    Parameter* params;
    Statement* body;
};

struct Statement
{
    uint8_t type;

    union
    {
        Expression* expr;

        struct
        {
            strptr    name;
            Function* ptr;
        } function;

        struct
        {
            strptr      name;
            Variable*   type;
            Expression* value;
        } variable;

        struct
        {
            Expression* condition;
            Statement*  body;
        } if_stmt;

        struct
        {
            Expression* condition;
            Statement*  body;
        } while_stmt;

        struct
        {
            Statement*  variable;
            Expression* condition;
            Expression* step;
            Statement*  body;
        } for_stmt;

        struct
        {
            Expression* expression;
        } ret_stmt;

        struct
        {
            strptr     name;
            Structure* structure;
        } struct_def;
    };

    Statement* next;
};

struct AST
{
    Statement* statements;
};

#endif // AST_HPP

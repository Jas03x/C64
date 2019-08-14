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

struct Identifier
{
	strptr      str;
	Identifier* next;
};

enum
{
    EXPR_INVALID     = 0x0,
    EXPR_SUB_EXPR    = 0x1,
    EXPR_LITERAL     = 0x2,
    EXPR_IDENTIFIER  = 0x3,
    EXPR_ASSIGN      = 0x4,
    EXPR_OPERATION   = 0x5,
    EXPR_INITIALIZER = 0x6
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
    EXPR_OP_INDEX                  = 0x19,
	EXPR_OP_FUNCTION_CALL          = 0x1A
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

		Identifier* identifier;

        struct
        {
            Expression* lhs;
            Expression* rhs;
        } assign;

        struct
        {
            uint8_t     op;
			union
			{
				struct
				{
					Expression* lhs;
					Expression* rhs;
				};

				struct
				{
					Expression* function;
					Argument*   arguments;
				} call;
			};
        } operation;

        Initializer initializer;
    };
};

enum
{
    STMT_INVALID       = 0x00,
    STMT_FUNCTION_DEF  = 0x01,
    STMT_FUNCTION_DECL = 0x02,
    STMT_EXPR          = 0x03,
    STMT_VARIABLE_DECL = 0x04,
    STMT_IF            = 0x05,
    STMT_ELSE_IF       = 0x06,
    STMT_ELSE          = 0x07,
    STMT_RET           = 0x08,
    STMT_STRUCT_DEF    = 0x09,
    STMT_STRUCT_DECL   = 0x0A,
    STMT_FOR           = 0x0B,
    STMT_WHILE         = 0x0C,
    STMT_NAMESPACE     = 0x0D,
    STMT_TYPEDEF       = 0x0E,
    STMT_COMPOUND_STMT = 0x0F,
    STMT_BREAK         = 0x10,
    STMT_CONTINUE      = 0x11,
    STMT_GOTO          = 0x12,
    STMT_SWITCH        = 0x13,
    STMT_CASE          = 0x14,
    STMT_LABEL         = 0x15,
    STMT_DEFAULT_CASE  = 0x16,
    STMT_ENUM_DEF      = 0x17,
    STMT_IMPORT        = 0x18,
    STMT_EXPORT        = 0x19,
    STMT_MODULE        = 0x20
};

enum TYPE
{
    TYPE_UNKNOWN = 0x00,
    TYPE_VOID    = 0x01,
    TYPE_U8      = 0x02,
    TYPE_U16     = 0x03,
    TYPE_U32     = 0x04,
    TYPE_U64     = 0x05,
    TYPE_I8      = 0x06,
    TYPE_I16     = 0x07,
    TYPE_I32     = 0x08,
    TYPE_I64     = 0x09,
    TYPE_F32     = 0x0A,
    TYPE_F64     = 0x0B,
    TYPE_PTR     = 0x0C,
    TYPE_STRUCT  = 0x0D,
    TYPE_ENUM    = 0x0E,
	TYPE_FUNCTION_POINTER     = 0x0F,
    TYPE_CONSTANT_SIZED_ARRAY = 0x10,
    TYPE_VARIABLE_SIZED_ARRAY = 0x11
};

struct Variable;

struct Structure
{
    strptr name;

    bool is_union;

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

struct Enum
{
    strptr name;
    
    struct Value
    {
        strptr  name;
        Literal value;

        Value* next;
    };

    Value* values;
};

struct Parameter;

struct Variable
{
    uint8_t type;
    VariableFlags flags;

    union
    {
        Identifier* identifier;

        struct
        {
            Expression*     size;
            const Variable* elements;
        } array;

        Enum*      enumerator;
        Structure* structure;

		struct
		{
			Variable*  ret_type;
			Parameter* parameters;
		} func_ptr;

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

enum ACCESS
{
    PRIVATE = 0,
    PUBLIC  = 1
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
            Statement* body;
        } else_stmt;

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

        struct
        {
            Identifier* identifier;
            Statement*  statements;
        } name_space;

        struct
        {
            Identifier* identifier;
            Variable*   variable;
        } type_def;

        struct
        {
            Statement* body;
        } compound_stmt;

        struct
        {
            strptr target;
        } goto_stmt;

        struct
        {
            strptr name;
        } label;

        struct
        {
            Expression* expr;
            Statement*  body;
        } switch_stmt;

        struct
        {
            Literal    value;
            Statement* body;
        } case_stmt;

        struct
        {
            strptr name;
            Enum*  enumerator;
        } enum_def;

        struct
        {
            strptr module_name;
        } import_stmt;

        struct
        {
            strptr module_name;
        } export_stmt;

        struct
        {
            uint8_t access;
            strptr  name;
        } module_decl;

    };

    Statement* next;
};

struct AST
{
    Statement* statements;
};

#endif // AST_HPP

#ifndef AST_HPP
#define AST_HPP

#include <literal.hpp>
#include <token.hpp>

#include <util/list.hpp>

enum
{
    EXPR_INVALID          = 0x0,
    EXPR_SUB_EXPR         = 0x1,
    EXPR_LITERAL          = 0x2,
    EXPR_IDENTIFIER       = 0x3,
    EXPR_OPERATION        = 0x4,
    EXPR_INITIALIZER      = 0x5,
	EXPR_FUNCTION_CALL    = 0x6,
	EXPR_STATIC_CAST      = 0x7,
	EXPR_REINTERPRET_CAST = 0x8
};

enum
{
    EXPR_OP_INVALID                = 0x00,
    EXPR_OP_ADD                    = 0x01,
    EXPR_OP_SUB                    = 0x02,
    EXPR_OP_MUL                    = 0x03,
    EXPR_OP_DIV                    = 0x04,
	EXPR_OP_MOD                    = 0x05,
	EXPR_OP_INCREMENT              = 0x06,
	EXPR_OP_DECREMENT			   = 0x07,
    EXPR_OP_LOGICAL_NOT            = 0x08,
    EXPR_OP_LOGICAL_AND            = 0x09,
    EXPR_OP_LOGICAL_OR             = 0x0A,
    EXPR_OP_BITWISE_COMPLEMENT     = 0x0B,
    EXPR_OP_BITWISE_XOR            = 0x0C,
    EXPR_OP_BITWISE_AND            = 0x0D,
    EXPR_OP_BITWISE_OR             = 0x0E,
    EXPR_OP_BITWISE_L_SHIFT        = 0x0F,
    EXPR_OP_BITWISE_R_SHIFT        = 0x10,
    EXPR_OP_CMP_EQUAL              = 0x11,
    EXPR_OP_CMP_NOT_EQUAL          = 0x12,
    EXPR_OP_CMP_LESS_THAN          = 0x13,
    EXPR_OP_CMP_MORE_THAN          = 0x14,
    EXPR_OP_CMP_LESS_THAN_OR_EQUAL = 0x15,
    EXPR_OP_CMP_MORE_THAN_OR_EQUAL = 0x16,
    EXPR_OP_REFERENCE              = 0x17,
    EXPR_OP_DEREFERENCE            = 0x18,
    EXPR_OP_ASSIGN                 = 0x19,
    EXPR_OP_ACCESS_FIELD           = 0x1A,
    EXPR_OP_ARROW                  = 0x1B,
    EXPR_OP_INDEX                  = 0x1C
};

enum
{
    STMT_INVALID       = 0x00,
    STMT_FUNCTION_DEF  = 0x01,
    STMT_FUNCTION_DECL = 0x02,
    STMT_EXPR          = 0x03,
    STMT_VARIABLE_DECL = 0x04,
    STMT_IF            = 0x05,
    STMT_ELSE          = 0x06,
    STMT_RETURN        = 0x07,
    STMT_COMP_DEF      = 0x08,
    STMT_COMP_DECL     = 0x09,
    STMT_FOR           = 0x0A,
    STMT_WHILE         = 0x0B,
    STMT_TYPEDEF       = 0x0C,
    STMT_COMPOUND_STMT = 0x0D,
    STMT_BREAK         = 0x0E,
    STMT_CONTINUE      = 0x0F,
    STMT_GOTO          = 0x10,
    STMT_SWITCH        = 0x11,
    STMT_CASE          = 0x12,
    STMT_LABEL         = 0x13,
    STMT_DEFAULT_CASE  = 0x14,
    STMT_ENUM_DEF      = 0x15,
    STMT_ENUM_DECL     = 0x16
};

enum TYPE
{
    TYPE_UNKNOWN    = 0x00,
    TYPE_VOID       = 0x01,
    TYPE_U8         = 0x02,
    TYPE_U16        = 0x03,
    TYPE_U32        = 0x04,
    TYPE_U64        = 0x05,
    TYPE_I8         = 0x06,
    TYPE_I16        = 0x07,
    TYPE_I32        = 0x08,
    TYPE_I64        = 0x09,
    TYPE_F32        = 0x0A,
    TYPE_F64        = 0x0B,
    TYPE_PTR        = 0x0C,
    TYPE_COMPOSITE  = 0x0D,
    TYPE_ENUMERATOR = 0x0E,
	TYPE_FUNCTION_POINTER     = 0x0F,
    TYPE_CONSTANT_SIZED_ARRAY = 0x10,
    TYPE_VARIABLE_SIZED_ARRAY = 0x11
};

enum COMPOSITE_TYPE
{
    COMP_TYPE_INVALID = 0,
    COMP_TYPE_STRUCT  = 1,
    COMP_TYPE_UNION   = 2
};

struct Expression;
struct Statement;
struct Type;

struct Composite
{
    uint8_t type;
    strptr  name;
    List<Statement> body;
};

struct Enumerator
{
    strptr name;
    
    struct Value
    {
        strptr      name;
        Expression* value;
    };

    List<Value> values;
};

struct Parameter
{
    strptr name;
    Type*  type;
};

struct Function
{
    strptr name;
    Type*  ret_type;
    List<Parameter> parameters;
    List<Statement> body;
};

union TypeFlags
{
    struct
    {
        unsigned int is_constant        : 1;
        unsigned int is_external_symbol : 1;
    } bits;

    uint8_t all;
};

struct Type
{
    uint8_t type;
    TypeFlags flags;

    union
    {
        Enumerator* enumerator;
        Composite*  composite;
        strptr      identifier;
        Type*       pointer;

        struct
        {
            Expression* size;
            Type*   elements;
        } array;

		struct
		{
			Type*  ret_type;
			List<Parameter> parameters;
		} func_ptr;
    } data;
};

struct Expression
{
    uint8_t type;

    union
    {
        Literal     literal;
        Expression* sub_expr;
		strptr      identifier;
        List<Expression*> initializer;

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
			};
        } operation;
		
		struct
		{
			Type* type;
			Expression* expr;
		} cast;

		struct
		{
			Expression* function;
			List<Expression> arguments;
		} function_call;
    } data;
};

struct Statement
{
    uint8_t type;

    union
    {
        Expression* expr;
        Function*   function;
        Composite*  composite;
        Enumerator* enumerator;

        struct
        {
            strptr      name;
            Type*       type;
            Expression* value;
        } variable_decl;

        struct
        {
            Expression* condition;
            List<Statement> body;
            
            Statement*  else_stmt;
        } if_stmt;

        struct
        {
            List<Statement> body;
        } else_stmt;

        struct
        {
            Expression* condition;
            List<Statement> body;
        } while_stmt;

        struct
        {
            Statement*  variable;
            Expression* condition;
            Expression* step;
            List<Statement> body;
        } for_stmt;

        struct
        {
            List<Statement> statements;
        } compound_stmt;

        struct
        {
            strptr  name;
            Type*   variable;
        } type_def;

        struct
        {
            Expression* expression;
        } ret_stmt;

        struct
        {
            strptr target;
        } goto_stmt;

        struct
        {
            Expression* expr;
            Statement*  body;
        } switch_stmt;

        struct
        {
            Expression* value;
        } case_stmt;

        struct
        {
            strptr name;
        } label;
    } data;
};

struct AST
{
    List<Statement> statements;
};

void delete_ast(AST* ast);

#endif // AST_HPP

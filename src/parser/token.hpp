#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <stdint.h>

#include <strptr.hpp>
#include <literal.hpp>

enum TOKEN
{
    TK_INVALID,
    TK_CONST,
    TK_EXTERN,
    TK_STRUCT,
    TK_RETURN,
    TK_IF,
    TK_EQUAL,
    TK_LEFT_ARROW_HEAD,
    TK_RIGHT_ARROW_HEAD,
    TK_PLUS,
    TK_MINUS,
    TK_DOT,
    TK_ASTERISK,
    TK_FORWARD_SLASH,
    TK_OPEN_CURLY_BRACKET,
    TK_CLOSE_CURLY_BRACKET,
    TK_OPEN_ROUND_BRACKET,
    TK_CLOSE_ROUND_BRACKET,
    TK_OPEN_SQUARE_BRACKET,
    TK_CLOSE_SQUARE_BRACKET,
    TK_SEMICOLON,
    TK_LITERAL,
    TK_IDENTIFIER,
    TK_COMMA,
    TK_OR,
    TK_AND,
    TK_CARET,
    TK_EXPLANATION_MARK,
    TK_AMPERSAND,
    TK_PERCENT,
    TK_NAMESPACE,
    TK_TYPE,
    TK_FOR,
    TK_WHILE,
	TK_COLON,
    TK_TYPEDEF,
    TK_BREAK,
    TK_GOTO,
    TK_ELSE,
    TK_CONTINUE,
    TK_SWITCH,
    TK_CASE,
    TK_DEFAULT,
    TK_ENUM,
    TK_EOF,
    TK_COUNT
};

enum
{
    TK_TYPE_INVALID = 0x0,
    TK_TYPE_VOID    = 0x1,
    TK_TYPE_U8      = 0x2,
    TK_TYPE_U16     = 0x3,
    TK_TYPE_U32     = 0x4,
    TK_TYPE_U64     = 0x5,
    TK_TYPE_I8      = 0x6,
    TK_TYPE_I16     = 0x7,
    TK_TYPE_I32     = 0x8,
    TK_TYPE_I64     = 0x9,
    TK_TYPE_F32     = 0xA,
    TK_TYPE_F64     = 0xB
};

struct Token
{
    uint8_t type;

    union
    {
        uint8_t subtype;

        Literal literal;

        struct
        {
            strptr string;
        } identifier;
    };
};

#endif // TOKEN_HPP

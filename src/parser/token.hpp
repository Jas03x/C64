#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <stdint.h>

#include <type.hpp>
#include <strptr.hpp>
#include <literal.hpp>

enum TOKEN
{
    TK_INVALID,
    TK_CONST,
    TK_RETURN,
    TK_IF,
    TK_TYPE,
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
    TK_EOF,
    TK_COUNT
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

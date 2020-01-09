#ifndef TOKEN_STACK_HPP
#define TOKEN_STACK_HPP

#include <vector>

#include <token.hpp>

class TokenStack
{
private:
    unsigned int m_position;
    std::vector<Token> m_tokens;

public:
    TokenStack();

    const Token& peek();
    const Token& pop();

    void push(const Token& tk);
};

#endif // TOKEN_STACK_HPP
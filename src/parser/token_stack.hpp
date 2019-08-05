#ifndef TOKEN_STACK_HPP
#define TOKEN_STACK_HPP

#include <vector>

#include <token.hpp>

class TokenStack
{
private:
    unsigned int       m_index;
    std::vector<Token> m_stack;

public:
    TokenStack();

    void push(const Token& tk);
    Token pop();
    Token peek(unsigned int offset);
    void clear();
};

#endif // TOKEN_STACK_HPP
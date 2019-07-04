#ifndef STACK_HPP
#define STACK_HPP

#include <vector>

#include <token.hpp>

class Stack
{
private:
    unsigned int       m_index;
    std::vector<Token> m_stack;

public:
    Stack();

    void push(const Token& tk);
    Token pop();
    Token peek(unsigned int offset);
    void clear();
};

#endif // STACK_HPP
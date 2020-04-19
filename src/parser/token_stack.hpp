#ifndef TOKEN_STACK_HPP
#define TOKEN_STACK_HPP

#include <map>
#include <vector>

#include <token.hpp>

class TokenStack
{
private:
    unsigned int m_position;
    std::vector<Token> m_tokens;

    std::map<strptr, strptr> m_identifier_set;

public:
    TokenStack();

    void insert_identifier(const strptr& ptr);
    const char* find_identifier(const strptr& ptr);

    const Token& pop();
    const Token& peek();
    
    void push(const Token& tk);

    const std::vector<Token>& get_tokens();
};

#endif // TOKEN_STACK_HPP

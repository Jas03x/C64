#include <token_stack.hpp>

TokenStack::TokenStack()
{
    m_position = 0;
}

#include <debug.hpp>
const Token& TokenStack::pop()
{
    Token tk = m_tokens[m_position++];
    printf("consume: ");
    print_token(tk);
    return tk;
    //return m_tokens[m_position++];
}

const Token& TokenStack::peek()
{
    return m_tokens[m_position];
}

void TokenStack::push(const Token& tk)
{
    m_tokens.push_back(tk);
}

void TokenStack::insert_identifier(const strptr& ptr)
{
    m_identifier_set[ptr] = ptr;
}

const char* TokenStack::find_identifier(const strptr& ptr)
{
    const char* str = nullptr;

    std::map<strptr, strptr>::const_iterator it = m_identifier_set.find(ptr);
    if(it != m_identifier_set.end())
    {
        str = it->second.ptr;
    }

    return str;
}

const std::vector<Token>& TokenStack::get_tokens()
{
    return m_tokens;
}

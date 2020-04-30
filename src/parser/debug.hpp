#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdio.h>
#include <token.hpp>

#define error(str, ...) printf("[%s]: " str, __FUNCTION__, ##__VA_ARGS__);

void print_token(const Token& tk);

#endif // DEBUG_HPP
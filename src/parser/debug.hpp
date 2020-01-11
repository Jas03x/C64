#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdio.h>

#define error(str, ...) printf("[%s]: " str, __FUNCTION__, ##__VA_ARGS__);

#include <ast.hpp>
#include <token.hpp>

void print_token(const Token& tk);

#endif // DEBUG_HPP
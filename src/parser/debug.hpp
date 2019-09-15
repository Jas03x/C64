#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdio.h>

#define error(str, ...) printf("[%s]: " str, __FUNCTION__, ##__VA_ARGS__);

#include <ast.hpp>
#include <token.hpp>

#endif // DEBUG_HPP
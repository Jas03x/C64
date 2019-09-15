#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdio.h>

#define error(str, ...) printf("[%s]: " str, __FUNCTION__, ##__VA_ARGS__);

#include <ast.hpp>
#include <token.hpp>

void print_ast(AST* ast);
void print_statement(Statement* stmt, unsigned int level);
void print_function(Function* func, unsigned int level);
void print_variable(Variable* var, unsigned int level);
void print_parameter(Parameter* param, unsigned int level);
void print_composite(Composite* composite, unsigned int level);
void print_enumerator(Enumerator* enumerator, unsigned int level);
void print_expression(Expression* expr, unsigned int level);

#endif // DEBUG_HPP
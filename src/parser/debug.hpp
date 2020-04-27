#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdio.h>

#define error(str, ...) printf("[%s]: " str, __FUNCTION__, ##__VA_ARGS__);

#include <ast.hpp>
#include <token.hpp>

void print_expr(unsigned int level, Expression* expr);
void print_parameter_list(unsigned int level, const List<Parameter>* list);
void print_identifier(unsigned int level, const strptr* id);
void print_type(unsigned int level, Type* type);
void print_function_decl(unsigned int level, Function* func);
void print_function_def(unsigned int level, Function* func);
void print_statement(unsigned int level, Statement* stmt);

void print_token(const Token& tk);
void print_ast(const AST& ast);

#endif // DEBUG_HPP
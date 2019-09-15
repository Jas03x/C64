#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdio.h>

#define error(str, ...) printf("[%s]: " str, __FUNCTION__, ##__VA_ARGS__);

#include <ast.hpp>
#include <token.hpp>

void debug_indent(unsigned int level);
void debug_print_expr(Expression* expr, unsigned int level = 0);
void debug_print_variable(const Variable* var, unsigned int level = 0);
void debug_print_composite(const Composite* composite, unsigned level = 0);
void debug_print_stmt(Statement* stmt, unsigned int level = 0);
void debug_print_token(const Token& tk);
void debug_print_namespace(const Statement* statment, unsigned int level = 0);
void debug_print_typedef(const Statement* statement, unsigned int level = 0);
void debug_print_switch(const Statement* statement, unsigned int level = 0);
void debug_print_case(const Statement* statement, unsigned int level = 0);
void debug_print_enum(const Enum* enumerator, unsigned int level = 0);

void debug_print_ast(AST* ast);

#endif // DEBUG_HPP
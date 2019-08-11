#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <ast.hpp>
#include <token.hpp>

void debug_indent(unsigned int level);
void debug_print_expr(Expression* expr, unsigned int level = 0);
void debug_print_variable(const Variable* var, unsigned int level = 0);
void debug_print_struct(const Structure* structure, unsigned level = 0);
void debug_print_stmt(Statement* stmt, unsigned int level = 0);
void debug_print_struct_member(Structure::Member* member, unsigned int level);
void debug_print_token(const Token& tk);
void debug_print_namespace(const Statement* statment, unsigned int level = 0);
void debug_print_typedef(const Statement* statement, unsigned int level = 0);

void debug_print_ast(AST* ast);

#endif // DEBUG_HPP
#ifndef AST_PRINTER_HPP
#define AST_PRINTER_HPP

#include <vector>

#include <ast.hpp>

class AST_Printer
{
private:
    std::vector<uint8_t> m_tab_stack;

    enum TAB
    {
        SPACE = 0,
        LINE  = 1,
        JOINT = 2
    };

private:
    AST_Printer();

    void print(const char* format, ...);

    void print_identifier(unsigned int indent, const strptr* id);
    void print_body(unsigned int indent, const List<Statement>* body);

    void print_statment(unsigned int indent, const Statement* stmt);

    void print_expr(unsigned int indent, const Expression* expr);
    void print_expr_operation(unsigned int indent, const Expression* lhs, const Expression* rhs);
    void print_func_call(unsigned int indent, const Expression::Func_Call* expr);

    void print_literal(unsigned int indent, const Literal* literal);
    void print_array(unsigned int indent, const Type::Array* array);
    void print_parameter(unsigned int indent, const Function::Parameter* param);
    void print_parameter_list(unsigned int indent, const List<Function::Parameter>* list);
    void print_type(unsigned int indent, const Type* type);
    void print_type_flags(unsigned int indent, const Type::Flags* flags);
    void print_function_decl(unsigned int indent, const Function* func);
    void print_function_def(unsigned int indent, const Function* func);

public:
    static void Print(const AST* ast);
};

#endif // AST_PRINTER_HPP
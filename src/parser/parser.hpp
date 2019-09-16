#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>

#include <ast.hpp>
#include <symbol_table.hpp>
#include <token_stack.hpp>
#include <expression_list.hpp>
#include <expression_stack.hpp>

class Parser
{
private:
    SymbolTable    m_symbols;
	TokenStack*    m_stack;
	ExpressionList m_list;

private:
    Parser(TokenStack* stack);
    
    uint8_t scan_identifier();

    // stmt:
    bool parse_body(list& stmt_list);
    bool parse_statement(list& stmt_list);    
    bool parse_definition(list& stmt_list);
    bool parse_declaration(list& stmt_list);
    bool parse_expression(list& stmt_list);
    bool parse_typedef(list& stmt_list);
    bool parse_composite_definition(list& stmt_list);
    bool parse_enumerator_definition(list& stmt_list);
    bool parse_function_declaration(list& stmt_list, Variable* var, strptr name);
    bool parse_variable_declaration(list& stmt_list, Variable* type, Variable* var, strptr name);
    bool parse_namespace(list& stmt_list);
    bool parse_if_statement(list& stmt_list);
    bool parse_if_statement(Statement** ptr);
    bool parse_return_statement(list& stmt_list);

    // type:
    bool parse_variable(Variable** var);
    bool parse_variable_modifiers(VariableFlags& flags);
    bool parse_composite(Composite** ptr);
    bool parse_enumerator(Enumerator** ptr);
    bool parse_pointer(Variable** ptr);
    bool parse_array(Variable** ptr);
    bool parse_identifier(Identifier** ptr);
    bool parse_parameter(Parameter** ptr);

    // expr:
    bool parse_expression(Expression** ptr);

public:
    static AST* Parse(TokenStack* stack);
};

#endif // PARSER_HPP

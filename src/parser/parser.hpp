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
    bool parse_body(list& statements);
    bool parse_statement(list& statements);    
    bool parse_definition(list& statements);
    bool parse_expression(list& statements);
    bool parse_typedef(list& statements);
    bool parse_variable_definition(list& statements);
    bool parse_composite_definition(list& statements);
    bool parse_enumerator_definition(list& statements);

    // type:
    bool parse_variable(Variable** var);
    bool parse_variable_modifiers(VariableFlags& flags);
    bool parse_composite(Composite** ptr);
    bool parse_enumerator(Enumerator** ptr);
    bool parse_pointer_array(Variable** ptr);

    // expr:
    bool parse_expression(Expression** ptr);

public:
    static AST* Parse(TokenStack* stack);
};

#endif // PARSER_HPP

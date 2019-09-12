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
    ~Parser();

	bool process_symbol(Statement* stmt);
    bool scan_identifier(SymbolTable::Entry** ptr);

    AST* parse();

    bool parse_statement(Statement** ptr);
    
	bool parse_operator(Expression** ptr);
	bool parse_sub_expr(Expression** ptr);
    bool parse_expression(Expression** ptr);
	
	uint8_t get_operator_precedence(uint8_t op);
	bool process_expression(Expression** lhs, ExpressionStack* stack, uint8_t minimum_precedence);

    bool parse_modifiers(VariableFlags& flags);
    bool parse_arguments(Argument** args);
    bool parse_parameters(Parameter** params);
    bool parse_array(Variable** variable);
    bool parse_initializer(Expression** ptr);

    bool parse_enumerator(Enum** ptr);
    bool parse_enum_value(Enum::Value** ptr);
    bool parse_enum_definition(Statement** ptr);

    bool parse_composite(Composite** composite);
    bool parse_composite_definition(Statement** ptr);

    bool parse_def_or_decl(Statement** ptr);
    bool parse_declaration(Statement** stmt);
    bool parse_definition(Statement** stmt);

	bool parse_cast(Expression** ptr);
	bool parse_function_pointer(strptr& name, Variable** ptr, Variable* ret_type);
    bool parse_switch_stmt(Statement** ptr);
    bool parse_default_stmt(Statement** ptr);
    bool parse_case_stmt(Statement** ptr);
    bool parse_continue_stmt(Statement** ptr);
    bool parse_label(Statement** ptr);
    bool parse_goto_stmt(Statement** ptr);
    bool parse_break_stmt(Statement** ptr);
    bool parse_compound_stmt(Statement** ptr);
    bool parse_typedef(Statement** ptr);
    bool parse_namespace(Statement** ptr);
	bool parse_identifier(Identifier** identifier);
    bool parse_body(Statement** stmt);
    bool parse_for_stmt(Statement** ptr);
    bool parse_while_stmt(Statement** ptr);
    bool parse_variable(Variable** ptr);
    bool parse_if_stmt(Statement** stmt);
    bool parse_else_stmt(Statement** ptr);
    bool parse_return(Statement** stmt);
    bool parse_function_decl(Variable* var, strptr name, Statement** ptr);
    bool parse_variable_decl(Variable* var, strptr name, Statement** ptr);

public:
    static AST* Parse(TokenStack* stack);
};

#endif // PARSER_HPP

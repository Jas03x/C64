#ifndef PARSER_HPP
#define PARSER_HPP

#include <map>
#include <string>

#include <token_stack.hpp>
#include <expression_list.hpp>

typedef std::map<std::string, Symbol*> SymbolMap;

class Parser
{
private:
    enum
    {
        PRECEDENCE_LEVEL_INVALID =  0,
        PRECEDENCE_LEVEL_1       =  1,
        PRECEDENCE_LEVEL_2       =  2,
        PRECEDENCE_LEVEL_3       =  3,
        PRECEDENCE_LEVEL_4       =  4,
        PRECEDENCE_LEVEL_5       =  5,
        PRECEDENCE_LEVEL_6       =  6,
        PRECEDENCE_LEVEL_7       =  7,
        PRECEDENCE_LEVEL_8       =  8,
        PRECEDENCE_LEVEL_9       =  9,
        PRECEDENCE_LEVEL_MAX     = 10
    };

private:
    TokenStack*    m_stack;
    ExpressionList m_list;

    SymbolMap m_symbols;

private:
    Parser(TokenStack* stack);
    ~Parser();

    Symbol* find_symbol(strptr id);
    bool    insert_symbol(strptr id, Symbol* type);
    bool    process_symbol(Statement* stmt, Symbol** sym_ptr);

    bool check_operator_precedence(unsigned int precedence_level, uint8_t op);

    AST* parse();

    bool process_expression(ExpressionList::Entry* list, Expression** expr);

    // checks if the identifier exists and its type
    // -1: error
    //  0: false
    //  1: true
    int is_type(strptr str);

    bool parse_statement(Statement** ptr);
    bool parse_value(Expression** ptr);
    bool parse_operator(Expression** ptr);
    bool parse_expression(Expression** ptr);

    bool parse_modifiers(VariableFlags& flags);
    bool parse_arguments(Argument** args);
    bool parse_parameters(Parameter** params);

    bool parse_body(Statement** stmt);
    bool parse_variable(Variable** ptr);
    bool parse_if_stmt(Statement** stmt);
    bool parse_return(Statement** stmt);
    bool parse_declaration(Statement** stmt);
    bool parse_struct_declaration(Statement** ptr);
    bool parse_function_decl(Variable* var, strptr name, Statement** ptr);
    bool parse_variable_decl(Variable* var, strptr name, Statement** ptr);

public:
    static AST* Parse(TokenStack* stack);

};

#endif // PARSER_HPP

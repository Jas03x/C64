#include <debug.hpp>

#include <stdarg.h>

void indent(unsigned int level)
{
    for(unsigned int i = 0; i < level; i++) {
        printf("    ");
    }
}

void print(unsigned int level, const char* format, ...)
{
    for(unsigned int i = 0; i < level; i++) {
        printf("\t");
    }

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

const char* token_to_str(const Token& tk)
{
    const static char* tbl[] =
    {
        "TK_CONST",
        "TK_EXTERN",
        "TK_STRUCT",
        "TK_RETURN",
        "TK_IF",
        "TK_EQUAL",
        "TK_LEFT_ARROW_HEAD",
        "TK_RIGHT_ARROW_HEAD",
        "TK_PLUS",
        "TK_MINUS",
        "TK_DOT",
        "TK_ASTERISK",
        "TK_FORWARD_SLASH",
        "TK_OPEN_CURLY_BRACKET",
        "TK_CLOSE_CURLY_BRACKET",
        "TK_OPEN_ROUND_BRACKET",
        "TK_CLOSE_ROUND_BRACKET",
        "TK_OPEN_SQUARE_BRACKET",
        "TK_CLOSE_SQUARE_BRACKET",
        "TK_SEMICOLON",
        "TK_LITERAL",
        "TK_IDENTIFIER",
        "TK_COMMA",
        "TK_OR",
        "TK_AND",
        "TK_CARET",
        "TK_EXPLANATION_MARK",
        "TK_AMPERSAND",
        "TK_PERCENT",
        "TK_NAMESPACE",
        "TK_TYPE",
        "TK_FOR",
        "TK_WHILE",
        "TK_COLON",
        "TK_TYPEDEF",
        "TK_BREAK",
        "TK_GOTO",
        "TK_ELSE",
        "TK_CONTINUE",
        "TK_SWITCH",
        "TK_UNION",
        "TK_CASE",
        "TK_DEFAULT",
        "TK_ENUM",
        "TK_STATIC_CAST",
        "TK_REINTERPRET_CAST",
        "TK_EOF"
    };

    const char* str = "TK_INVALID";
    if((tk.type > TK_INVALID) && (tk.type < TK_COUNT))
    {
        str = tbl[tk.type - 1];
    }
    return str;
}

void print_ast(AST* ast)
{
    for(list::element* it = ast->statements.head; it != nullptr; it = it->next)
    {
        print_statement(reinterpret_cast<Statement*>(it->ptr), 0);
    }
}

void print_statement(Statement* stmt, unsigned int level)
{
    switch(stmt->type)
    {
        case STMT_FUNCTION_DEF:  { print(level, "FUNCTION DEFINITION:\n");  print_function(stmt->function, level + 1); break; }
        case STMT_FUNCTION_DECL: { print(level, "FUNCTION DECLARATION:\n"); print_function(stmt->function, level + 1); break; }
        case STMT_EXPR:          { break; }
        case STMT_VARIABLE_DECL: { break; }
        case STMT_IF:            { break; }
        case STMT_RET:           { break; }
        case STMT_COMP_DEF:      { break; }
        case STMT_COMP_DECL:     { break; }
        case STMT_FOR:           { break; }
        case STMT_WHILE:         { break; }
        case STMT_NAMESPACE:     { print(level, "NAMESPACE:\n"); print_namespace(stmt, level + 1); break; }
        case STMT_TYPEDEF:       { break; }
        case STMT_COMPOUND_STMT: { break; }
        case STMT_BREAK:         { break; }
        case STMT_CONTINUE:      { break; }
        case STMT_GOTO:          { break; }
        case STMT_SWITCH:        { break; }
        case STMT_CASE:          { break; }
        case STMT_LABEL:         { break; }
        case STMT_DEFAULT_CASE:  { break; }
        case STMT_ENUM_DEF:      { break; }
        case STMT_ENUM_DECL:     { break; }
        default: { print(level, "INVALID STATEMENT\n"); break; }
    }
}

void print_function(Function* func, unsigned int level)
{
    print(level, "NAME: %.*s\n", func->name.len, func->name.ptr);
    
    print(level, "RETURN:\n");
    print_variable(func->ret_type, level + 1);

    print(level, "PARAMETERS:\n");
    for(list::element* it = func->parameters.head; it != nullptr; it = it->next)
    {
        print_parameter(reinterpret_cast<Parameter*>(it->ptr), level + 1);
    }

    print(level, "BODY:\n");
    for(list::element* it = func->body.head; it != nullptr; it = it->next)
    {
        print_statement(reinterpret_cast<Statement*>(it->ptr), level + 1);
    }
}

void print_variable(Variable* var, unsigned int level)
{
    switch(var->type)
    {
        case TYPE_PTR:
        {
            print(level, "PTR\n");
            print_variable(var->pointer, level + 1);
            break;
        }
        case TYPE_COMPOSITE:
        {
            print(level, "COMPOSITE\n");
            print_composite(var->composite, level + 1);
            break;
        }
        case TYPE_ENUMERATOR:
        {
            print(level, "ENUMERATOR\n");
            print_enumerator(var->enumerator, level + 1);
            break;
        }
        case TYPE_FUNCTION_POINTER:
        {
            print(level, "FUNCTION POINTER\n");
            
            print(level + 1, "RETURN:\n");
            print_variable(var->func_ptr.ret_type, level + 2);

            print(level + 1, "PARAMETERS:\n");
            for(list::element* it = var->func_ptr.parameters.head; it != nullptr; it++)
            {
                print_parameter(reinterpret_cast<Parameter*>(it->ptr), level + 1);
            }
            break;
        }
        case TYPE_CONSTANT_SIZED_ARRAY:
        {
            print(level, "FIXED SIZE ARRAY\n");

            print(level + 1, "SIZE:\n");
            print_expression(var->array.size, level + 2);

            print(level + 1, "ELEMENTS:\n");
            print_variable(var->array.elements, level + 2);
            break;
        }
        case TYPE_VARIABLE_SIZED_ARRAY:
        {
            print(level, "VARIABLE SIZE ARRAY\n");

            print(level + 1, "ELEMENTS:\n");
            print_variable(var->array.elements, level + 2);
            break;
        }
        default:
        {
            const char* type = "UNKNOWN";
            switch(var->type)
            {
                case TYPE_VOID: { type = "VOID"; break; }
                case TYPE_U8:   { type = "U8";   break; }
                case TYPE_U16:  { type = "U16";  break; }
                case TYPE_U32:  { type = "U32";  break; }
                case TYPE_U64:  { type = "U64";  break; }
                case TYPE_I8:   { type = "I8";   break; }
                case TYPE_I16:  { type = "I16";  break; }
                case TYPE_I32:  { type = "I32";  break; }
                case TYPE_I64:  { type = "I64";  break; }
                case TYPE_F32:  { type = "F32";  break; }
                case TYPE_F64:  { type = "F64";  break; }
                default:        { break; }
            }
            print(level, "TYPE: %s\n", type);
            break;
        }
    }
    
    print(level, "FLAGS: ");
    if(var->flags.is_constant)        { print(0, "CONSTANT "); }
    if(var->flags.is_external_symbol) { print(0, "EXTERNAL "); }
    print(0, "\n");
}

void print_composite(Composite* composite, unsigned int level)
{
    print(level, "%s:\n", (composite->type == COMP_TYPE_STRUCT) ? "STRUCT" : "UNION");

    print(level + 1, "NAME: %.*s\n", composite->name.len, composite->name.ptr);
    
    print(level + 1, "MEMBERS:\n");
    for(list::element* it = composite->body.head; it != nullptr; it = it->next)
    {
        print_statement(reinterpret_cast<Statement*>(it->ptr), level + 2);
    }
}

void print_parameter(Parameter* param, unsigned int level)
{
    print(level, "NAME: %.*s\n", param->name.len, param->name.ptr);
    
    print(level, "TYPE:\n");
    print_variable(param->type, level + 1);
}

void print_enumerator(Enumerator* enumerator, unsigned int level)
{
    print(level, "ENUMERATOR\n");

    print(level + 1, "NAME: %.*s\n", enumerator->name.len, enumerator->name.ptr);

    print(level + 1, "VALUES:\n");
    for(list::element* it = enumerator->values.head; it != nullptr; it = it->next)
    {
        Enumerator::Value* value = reinterpret_cast<Enumerator::Value*>(it->ptr);
        print(level + 2, "VALUE: %.*s\n", value->name.len, value->name.ptr);
        print_expression(value->value, level + 3);
    }
}

void print_expression(Expression* expr, unsigned int level)
{
    switch(expr->type)
    {
        case EXPR_SUB_EXPR:         { break; }
        case EXPR_LITERAL:          { break; }
        case EXPR_IDENTIFIER:       { break; }
        case EXPR_OPERATION:        { break; }
        case EXPR_INITIALIZER:      { break; }
        case EXPR_FUNCTION_CALL:    { break; }
        case EXPR_STATIC_CAST:      { break; }
        case EXPR_REINTERPRET_CAST: { break; }
        default: { print(level, "INVALID\n"); break; }
    }
}

void print_namespace(Statement* stmt, unsigned int level)
{
    print(level + 1, "NAME: %.*s\n", stmt->name_space.name.len, stmt->name_space.name.ptr);

    print(level + 1, "MEMBERS:\n");
    for(list::element* it = stmt->name_space.statements.head; it != nullptr; it = it->next)
    {
        print_statement(reinterpret_cast<Statement*>(it->ptr), level + 2);
    }
}

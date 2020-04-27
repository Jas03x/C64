#include <debug.hpp>

#include <stdarg.h>

#include <ascii.hpp>

void indent(unsigned int level)
{
    if (level > 0)
    {
        for (unsigned int i = 0; i < level - 1; i++) {
            printf("  |    ");
        }
        printf("  |--> ");
    }
}

void print(unsigned int level, const char* format, ...)
{
    indent(level);

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

void print_token(const Token& tk)
{
    if(tk.type == TK_TYPE)
    {
        const char* lookup_table[] = {
            "invalid", "void",
            "U8", "U16", "U32", "U64",
            "I8", "I16", "I32", "I64",
            "F32", "F64"
        };

        unsigned int subtype = (tk.data.subtype < TK_TYPE_COUNT) ? tk.data.subtype : 0;
        printf("type: %s\n", lookup_table[subtype]);
    }
    else if(tk.type == TK_LITERAL)
    {
        switch(tk.data.literal.type)
        {
            case LITERAL_INTEGER: { printf("literal: integer (%llu)\n", tk.data.literal.data.integer_value); break; }
            case LITERAL_FLOAT:   { printf("literal: decimal (%f)\n", tk.data.literal.data.float_value); break; }
            case LITERAL_CHAR:    { printf("literal: character (%c)\n", tk.data.literal.data.character); break; }
            case LITERAL_STRING:
            {
                printf("literal: string (%u, ", tk.data.literal.data.string.len);
                for(unsigned int i = 0; i < tk.data.literal.data.string.len; i++) {
                    char c = tk.data.literal.data.string.ptr[i];
                    if(IS_ALPHA_NUM(c) || (c == ' ')) {
                        printf("%c", c);
                    } else {
                        printf("\\x%hhX", c);
                    }
                }
                printf(")\n");
                break;
            }
            default: { printf("literal: invalid\n"); break; }
        }
    }
    else if(tk.type == TK_IDENTIFIER)
    {
        printf("identifier: %s\n", tk.data.identifier.ptr);
    }
    else
    {
        const char* str = "invalid";
        switch(tk.type)
        {
            case TK_CONST: { str = "const"; break; }
            case TK_EXTERN: { str = "extern"; break; }
            case TK_STRUCT: { str = "struct"; break; }
            case TK_RETURN: { str = "return"; break; }
            case TK_IF: { str = "if"; break; }
            case TK_EQUAL: { str = "="; break; }
            case TK_LEFT_ARROW_HEAD: { str = "<"; break; }
            case TK_RIGHT_ARROW_HEAD: { str = ">"; break; }
            case TK_PLUS: { str = "+"; break; }
            case TK_MINUS: { str = "-"; break; }
            case TK_DOT: { str = "."; break; }
            case TK_ASTERISK: { str = "*"; break; }
            case TK_FORWARD_SLASH: { str = "/"; break; }
            case TK_OPEN_CURLY_BRACKET: { str = "{"; break; }
            case TK_CLOSE_CURLY_BRACKET: { str = "}"; break; }
            case TK_OPEN_ROUND_BRACKET: { str = "("; break; }
            case TK_CLOSE_ROUND_BRACKET: { str = ")"; break; }
            case TK_OPEN_SQUARE_BRACKET: { str = "["; break; }
            case TK_CLOSE_SQUARE_BRACKET: { str = "]"; break; }
            case TK_SEMICOLON: { str = ";"; break; }
            case TK_COMMA: { str = ","; break; }
            case TK_OR: { str = "or"; break; }
            case TK_AND: { str = "and"; break; }
            case TK_CARET: { str = "^"; break; }
            case TK_EXPLANATION_MARK: { str = "!"; break; }
            case TK_AMPERSAND: { str = "&"; break; }
            case TK_VERTICAL_BAR: { str = "|"; break; }
            case TK_PERCENT: { str = "%"; break; }
            case TK_FOR: { str = "for"; break; }
            case TK_WHILE: { str = "while"; break; }
            case TK_COLON: { str = ":"; break; }
            case TK_TYPEDEF: { str = "typedef"; break; }
            case TK_BREAK: { str = "break"; break; }
            case TK_GOTO: { str = "goto"; break; }
            case TK_ELSE: { str = "else"; break; }
            case TK_CONTINUE: { str = "continue"; break; }
            case TK_SWITCH: { str = "switch"; break; }
            case TK_UNION: { str = "union"; break; }
            case TK_CASE: { str = "case"; break; }
            case TK_DEFAULT: { str = "default"; break; }
            case TK_ENUM: { str = "enum"; break; }
            case TK_STATIC_CAST: { str = "cast"; break; }
            case TK_REINTERPRET_CAST: { str = "r_cast"; break; }
            case TK_EOF: { str = "eof"; break; }
        }
        printf("%s\n", str);
    }
}

void print_expr(unsigned int level, Expression* expr)
{

}

void print_parameter(unsigned int level, const Parameter* param)
{
    print(level, "NAME:\n");
    print_identifier(level + 1, &param->name);
    print(level, "TYPE:\n");
    print_type(level + 1, param->type);
}

void print_parameter_list(unsigned int level, const List<Parameter>* list)
{
    for (List<Parameter>::Element* it = list->head; it != nullptr; it = it->next)
    {
        print(level, "PARAM:\n");
        print_parameter(level + 1, it->ptr);
    }
}

void print_identifier(unsigned int level, const strptr* id)
{
    if(id->len > 0)
    {
        print(level, "%.*s\n", id->len, id->ptr);
    }
    else
    {
        print(level, "NULL\n");
    }
}

void print_type(unsigned int level, Type* type)
{
    indent(level);
    if (type->flags.bits.is_external_symbol)
    {
        printf("EXTERN ");
    }
    if (type->flags.bits.is_constant)
    {
        printf("CONST ");
    }

    switch (type->type)
    {
        case TYPE_VOID: { printf("VOID\n"); break; }
        case TYPE_U8:   { printf("U8\n");   break; }
        case TYPE_U16:  { printf("U16\n");  break; }
        case TYPE_U32:  { printf("U32\n");  break; }
        case TYPE_U64:  { printf("U64\n");  break; }
        case TYPE_I8:   { printf("I8\n");   break; }
        case TYPE_I16:  { printf("I16\n");  break; }
        case TYPE_I32:  { printf("I32\n");  break; }
        case TYPE_I64:  { printf("I64\n");  break; }
        case TYPE_F32:  { printf("F32\n");  break; }
        case TYPE_F64:  { printf("F64\n");  break; }
        case TYPE_IDENTIFIER:
        {
            print_identifier(level + 1, &type->data.identifier);
            break;
        }
        case TYPE_PTR:
        {
            printf("PTR:\n");
            print_type(level + 1, type->data.pointer);
            break;
        }
        case TYPE_FUNC_PTR:
        {
            printf("FUNC PTR:\n");
            print(level + 1, "TYPE:\n");
            print_type(level + 2, type->data.func_ptr.ret_type);
            break;
        }
        case TYPE_ARRAY:
        {
            printf("ARRAY:\n");
            print(level + 1, "TYPE:\n");
            print_type(level + 2, type->data.array.elements);
            print(level + 1, "SIZE:\n");
            print_expr(level + 2, type->data.array.size);
            break;
        }
    }
}

void print_function_decl(unsigned int level, Function* func)
{
    print(level, "DECL:\n");
    print(level + 1, "TYPE:\n");
    print_type(level + 2, func->ret_type);
    print(level + 1, "PARAMETERS:\n");
    print_parameter_list(level + 2, &func->parameters);
}

void print_function_def(unsigned int level, Function* func)
{

}

void print_statement(unsigned int level, Statement* stmt)
{
    switch (stmt->type)
    {
        case STMT_FUNCTION_DECL: { print_function_decl(level, stmt->data.function); break; }
        case STMT_FUNCTION_DEF:  { print_function_def(level, stmt->data.function);  break; }
        default:
        {
            print(level, "UNKNOWN\n");
        }
    }
}

void print_ast(const AST& ast)
{
    for (List<Statement>::Element* it = ast.statements.head; it != nullptr; it = it->next)
    {
        print_statement(0, it->ptr);
    }
}

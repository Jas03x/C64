#include <ast_printer.hpp>

#include <stdarg.h>

const char* TAB_STR[] =
{
    "     ",
    " |   ",
    " +-- "
};

AST_Printer::AST_Printer()
{
}

void AST_Printer::print(const char* format, ...)
{
    for(unsigned int i = 0; i < m_tab_stack.size(); i++)
    {
        printf("%s", TAB_STR[m_tab_stack[i]]);
    }

    printf("%s", TAB_STR[TAB::JOINT]);

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

void AST_Printer::print_statment(unsigned int indent, const Statement* stmt)
{
    switch(stmt->type)
    {
        case STMT_FUNCTION_DEF:
        {
            print("DEF:\n");
            print_function_def(indent, stmt->data.function);
            break;
        }
        case STMT_FUNCTION_DECL:
        {
            print("DECL:\n");
            print_function_decl(indent, stmt->data.function);
            break;
        }
        case STMT_EXPR:
        {
            print("EXPR:\n");
            print_expr_stmt(indent, stmt);
            break;
        }
    }
}

void AST_Printer::print_expr_stmt(unsigned int indent, const Statement* stmt)
{
    m_tab_stack.push_back(indent);
    print_expr(TAB::LINE, stmt->data.expr);
    m_tab_stack.pop_back();
}

void AST_Printer::print_body(const List<Statement>* body)
{
    for(List<Statement>::Element* it = body->head; it != nullptr; it = it->next)
    {
        print_statment((it->next == nullptr) ? TAB::SPACE : TAB::LINE, it->ptr);
    }
}

void AST_Printer::print_expr(unsigned int indent, const Expression* expr)
{
    switch(expr->type)
    {
        case EXPR_FUNCTION_CALL:
        {
            print("FUNC CALL:\n");
            print_func_call(TAB::LINE, &expr->data.func_call);
            break;
        }
    }
}

void AST_Printer::print_func_call(unsigned int indent, const Expression::Func_Call* f_call)
{
    m_tab_stack.push_back(indent);

    print("FUNCTION:\n");
    print_expr(TAB::SPACE, f_call->function);

    print("ARGUMENTS:\n");
    for(List<Expression>::Element* it = f_call->arguments.head; it != nullptr; it = it->next)
    {
        print("ARG");
        print_expr(TAB::LINE, it->ptr);
    }

    m_tab_stack.pop_back();
}

void AST_Printer::print_parameter(unsigned int indent, const Function::Parameter* param)
{
    m_tab_stack.push_back(indent);

    print("NAME: %s\n", (param->name.len > 0) ? param->name.ptr : "NULL");
    print("DATATYPE:\n");
    print_type(TAB::SPACE, param->type);

    m_tab_stack.pop_back();
}

void AST_Printer::print_parameter_list(unsigned int indent, const List<Function::Parameter>* list)
{
    m_tab_stack.push_back(indent);

    for (List<Function::Parameter>::Element* it = list->head; it != nullptr; it = it->next)
    {
        print("PARAM:\n");
        print_parameter((it->next == nullptr) ? TAB::SPACE : TAB::LINE, it->ptr);
    }

    m_tab_stack.pop_back();
}

void AST_Printer::print_identifier(unsigned int indent, const strptr* id)
{
    m_tab_stack.push_back(indent);

    if(id->len > 0)
    {
        print("%.*s\n", id->len, id->ptr);
    }
    else
    {
        print("NULL\n");
    }

    m_tab_stack.pop_back();
}

void AST_Printer::print_array(unsigned int indent, const Type::Array* array)
{
    m_tab_stack.push_back(indent);

    print("DATATYPE:\n");
    print_type(TAB::LINE, array->elements);
    print("SIZE:\n");
    print_expr(TAB::SPACE, array->size);

    m_tab_stack.pop_back();
}

void AST_Printer::print_type_flags(unsigned int indent, const Type::Flags* flags)
{
    m_tab_stack.push_back(indent);

    if (flags->bits.is_external_symbol)
    {
        print("EXTERN\n");
    }
    if (flags->bits.is_constant)
    {
        print("CONST\n");
    }

    m_tab_stack.pop_back();
}

void AST_Printer::print_type(unsigned int indent, const Type* type)
{
    m_tab_stack.push_back(indent);

    if(type->flags.all == 0x0)
    {
        print("FLAGS: NONE\n");
    }
    else
    {
        print("FLAGS:\n");
        print_type_flags(TAB::LINE, &type->flags);
    }

    switch (type->type)
    {
        case TYPE_VOID: { print("TYPE: VOID\n"); break; }
        case TYPE_U8:   { print("TYPE: U8\n");   break; }
        case TYPE_U16:  { print("TYPE: U16\n");  break; }
        case TYPE_U32:  { print("TYPE: U32\n");  break; }
        case TYPE_U64:  { print("TYPE: U64\n");  break; }
        case TYPE_I8:   { print("TYPE: I8\n");   break; }
        case TYPE_I16:  { print("TYPE: I16\n");  break; }
        case TYPE_I32:  { print("TYPE: I32\n");  break; }
        case TYPE_I64:  { print("TYPE: I64\n");  break; }
        case TYPE_F32:  { print("TYPE: F32\n");  break; }
        case TYPE_F64:  { print("TYPE: F64\n");  break; }
        case TYPE_IDENTIFIER:
        {
            print("TYPE: IDENTIFIER\n");
            print_identifier(TAB::LINE, &type->data.identifier);
            break;
        }
        case TYPE_PTR:
        {
            print("TYPE: PTR\n");
            print_type(TAB::SPACE, type->data.pointer);
            break;
        }
        case TYPE_FUNC_PTR:
        {
            // TODO
            break;
        }
        case TYPE_ARRAY:
        {
            print("TYPE: ARRAY\n");
            print_array(TAB::LINE, type->data.array);
            break;
        }
    }

    m_tab_stack.pop_back();
}

void AST_Printer::print_function_decl(unsigned int indent, const Function* func)
{
    m_tab_stack.push_back(indent);

    print("NAME: %s\n", (func->name.len > 0) ? func->name.ptr : "NULL");

    print("DATATYPE:\n");
    print_type(TAB::LINE, func->ret_type);

    print("PARAMETERS:\n");
    print_parameter_list(TAB::SPACE, &func->parameters);

    m_tab_stack.pop_back();
}

void AST_Printer::print_function_def(unsigned int indent, const Function* func)
{
    m_tab_stack.push_back(indent);

    print("NAME: %s\n", (func->name.len > 0) ? func->name.ptr : "NULL");

    print("DATATYPE:\n");
    print_type(TAB::LINE, func->ret_type);

    print("PARAMETERS:\n");
    print_parameter_list(TAB::LINE, &func->parameters);

    print("BODY:\n");
    m_tab_stack.push_back(TAB::SPACE);
    print_body(&func->body);

    m_tab_stack.pop_back();
    m_tab_stack.pop_back();
}

void AST_Printer::Print(const AST* ast)
{
    AST_Printer printer;

    printf("AST:\n");
    printer.print_body(&ast->statements);
}

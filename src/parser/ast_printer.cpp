#include <ast_printer.hpp>

#include <stdarg.h>

const char* TAB_STR[] =
{
    "     ",
    " |   ",
    " +-- "
};

AST_Printer::AST_Printer(const AST* ast)
{
    printf("AST:\n");

    for(List<Statement>::Element* it = ast->statements.head; it != nullptr; it = it->next)
    {
        Statement* stmt = it->ptr;
        switch(stmt->type)
        {
            case STMT_FUNCTION_DEF:
            {
                break;
            }
            case STMT_FUNCTION_DECL:
            {
                print("DECL:\n");
                print_function_decl(TAB::LINE, stmt->data.function);
                break;
            }
        }
    }
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

void AST_Printer::print_expr(unsigned int indent, const Expression* expr)
{

}

void AST_Printer::print_parameter(unsigned int indent, const Function::Parameter* param)
{
    m_tab_stack.push_back(indent);

    print("NAME:\n");
    print_identifier(TAB::LINE, &param->name);
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

    if(flags->all == 0x0)
    {
        print("NONE\n");
    }
    else
    {
        if (flags->bits.is_external_symbol)
        {
            print("EXTERN\n");
        }
        if (flags->bits.is_constant)
        {
            print("CONST\n");
        }
    }

    m_tab_stack.pop_back();
}

void AST_Printer::print_type(unsigned int indent, const Type* type)
{
    m_tab_stack.push_back(indent);

    print("FLAGS:\n");
    print_type_flags(TAB::LINE, &type->flags);

    print("TYPE:\n");
    m_tab_stack.push_back(TAB::SPACE);

    switch (type->type)
    {
        case TYPE_VOID: { print("VOID\n"); break; }
        case TYPE_U8:   { print("U8\n");   break; }
        case TYPE_U16:  { print("U16\n");  break; }
        case TYPE_U32:  { print("U32\n");  break; }
        case TYPE_U64:  { print("U64\n");  break; }
        case TYPE_I8:   { print("I8\n");   break; }
        case TYPE_I16:  { print("I16\n");  break; }
        case TYPE_I32:  { print("I32\n");  break; }
        case TYPE_I64:  { print("I64\n");  break; }
        case TYPE_F32:  { print("F32\n");  break; }
        case TYPE_F64:  { print("F64\n");  break; }
        case TYPE_IDENTIFIER:
        {
            print("IDENTIFIER\n");
            print_identifier(TAB::LINE, &type->data.identifier);
            break;
        }
        case TYPE_PTR:
        {
            print("PTR:\n");
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
            print("ARRAY:\n");
            print_array(TAB::LINE, type->data.array);
            break;
        }
    }

    m_tab_stack.pop_back();
    m_tab_stack.pop_back();
}

void AST_Printer::print_function_decl(unsigned int indent, const Function* func)
{
    m_tab_stack.push_back(indent);

    print("DATATYPE:\n");
    print_type(TAB::LINE, func->ret_type);
    print("PARAMETERS:\n");
    print_parameter_list(TAB::SPACE, &func->parameters);

    m_tab_stack.pop_back();
}

void AST_Printer::print_function_def(unsigned int indent, const Function* func)
{

}

void AST_Printer::Print(const AST* ast)
{
    AST_Printer printer(ast);
}

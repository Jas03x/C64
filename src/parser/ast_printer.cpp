#include <ast_printer.hpp>

#include <stdarg.h>

#include <ascii.hpp>

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

void AST_Printer::print_statement(unsigned int indent, const Statement* stmt)
{
    switch(stmt->type)
    {
        case STMT_FUNCTION_DEF:
        {
            print("FUNCTION DEF:\n");
            print_function_def(indent, &stmt->data.function);
            break;
        }
        case STMT_FUNCTION_DECL:
        {
            print("FUNCTION DECL:\n");
            print_function_decl(indent, &stmt->data.function);
            break;
        }
        case STMT_VARIABLE_DEF:
        {
            print("VARIABLE DEF:\n");
            print_variable_def(indent, &stmt->data.variable);
            break;
        }
        case STMT_EXPR:
        {
            print("EXPR:\n");
            print_expr(indent, stmt->data.expr);
            break;
        }
        case STMT_FOR:
        {
            print("FOR:\n");
            print_for_stmt(indent, &stmt->data.for_loop);
            break;
        }
        case STMT_WHILE:
        {
        	print("WHILE:\n");
        	print_while_stmt(indent, &stmt->data.while_loop);
        	break;
        }
        case STMT_COMPOUND_STMT:
        {
            print("COMPOUND STMT:\n");
            print_body(TAB::SPACE, &stmt->data.compound_stmt);
            break;
        }
        case STMT_RETURN:
        {
            print("RETURN:\n");
            print_expr(indent, stmt->data.ret_stmt.expression);
            break;
        }
    }
}

void AST_Printer::print_for_stmt(unsigned int indent, const Statement::ForLoop* loop)
{
    m_tab_stack.push_back(indent);

    print("INIT:\n");
    m_tab_stack.push_back(indent);
    print_statement(TAB::SPACE, loop->init);
    m_tab_stack.pop_back();

    print("COND:\n");
    print_expr(TAB::LINE, loop->cond);

    print("STEP:\n");
    print_expr(TAB::LINE, loop->step);

    print("BODY:\n");
    m_tab_stack.push_back(TAB::SPACE);
    print_statement(TAB::SPACE, loop->body);
    m_tab_stack.pop_back();

    m_tab_stack.pop_back();
}

void AST_Printer::print_while_stmt(unsigned int indent, const Statement::WhileLoop* loop)
{
	m_tab_stack.push_back(indent);

	print("COND:\n");
	print_expr(TAB::LINE, loop->cond);

	print("BODY:\n");
	m_tab_stack.push_back(TAB::SPACE);
	print_statement(TAB::SPACE, loop->body);
	m_tab_stack.pop_back();

	m_tab_stack.pop_back();
}

void AST_Printer::print_body(unsigned int indent, const List<Statement>* body)
{
    m_tab_stack.push_back(indent);

    for(List<Statement>::Element* it = body->head; it != nullptr; it = it->next)
    {
        print_statement((it->next == nullptr) ? TAB::SPACE : TAB::LINE, it->ptr);
    }

    m_tab_stack.pop_back();
}

void AST_Printer::print_expr(unsigned int indent, const Expression* expr)
{
    if(expr->type == EXPR_LITERAL)
    {
        print_literal(TAB::SPACE, &expr->data.literal);
    }
    else if(expr->type == EXPR_IDENTIFIER)
    {
        print_identifier(TAB::SPACE, &expr->data.identifier);
    }
    else
    {
        m_tab_stack.push_back(indent);

        if(expr->type == EXPR_FUNCTION_CALL)
        {
            print("FUNC CALL:\n");
            print_func_call(TAB::SPACE, &expr->data.func_call);
        }
        else if(expr->type == EXPR_SUB_EXPR)
        {
            print("SUB-EXPR:\n");
            print_expr(TAB::SPACE, expr->data.sub_expr);
        }
        else if(expr->type == EXPR_OPERATION)
        {
            switch(expr->type)
            {
                case EXPR_OPERATION:
                {
                    switch(expr->data.operation.op)
                    {
                        case EXPR_OP_ADD:
                        {
                            print("ADD:\n");
                            print_expr_operation(TAB::SPACE, expr->data.operation.lhs, expr->data.operation.rhs);
                            break;
                        }
                        case EXPR_OP_SUB:
                        {
                            print("SUB:\n");
                            print_expr_operation(TAB::SPACE, expr->data.operation.lhs, expr->data.operation.rhs);
                            break;
                        }
                        case EXPR_OP_MUL:
                        {
                            print("MUL:\n");
                            print_expr_operation(TAB::SPACE, expr->data.operation.lhs, expr->data.operation.rhs);
                            break;
                        }
                        case EXPR_OP_DIV:
                        {
                            print("DIV:\n");
                            print_expr_operation(TAB::SPACE, expr->data.operation.lhs, expr->data.operation.rhs);
                            break;
                        }
                        case EXPR_OP_DEREFERENCE:
                        {
                            print("DEREFERENCE:\n");
                            print_expr_operation(TAB::SPACE, expr->data.operation.rhs);
                            break;
                        }
                        case EXPR_OP_ASSIGN:
                        {
                            print("ASSIGN:\n");
                            print_expr_operation(TAB::SPACE, expr->data.operation.lhs, expr->data.operation.rhs);
                            break;
                        }
                        case EXPR_OP_CMP_LESS_THAN:
                        {
                            print("LT:\n");
                            print_expr_operation(TAB::SPACE, expr->data.operation.lhs, expr->data.operation.rhs);
                            break;
                        }
                        case EXPR_OP_INCREMENT:
                        {
                            const Expression* lhs = expr->data.operation.lhs;
                            const Expression* rhs = expr->data.operation.rhs;
                            if(lhs != nullptr)
                            {
                                print("POST-INCREMENT:\n");
                                print_expr(TAB::SPACE, lhs);
                            }
                            else
                            {
                                print("PRE-INCREMENT:\n");
                                print_expr(TAB::SPACE, rhs);
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
        
        m_tab_stack.pop_back();
    }
}

void AST_Printer::print_expr_operation(unsigned int indent, const Expression* expr)
{
    m_tab_stack.push_back(indent);

    print_expr(TAB::SPACE, expr);

    m_tab_stack.pop_back();
}

void AST_Printer::print_expr_operation(unsigned int indent, const Expression* lhs, const Expression* rhs)
{
    m_tab_stack.push_back(indent);

    print("LHS:\n");
    print_expr(TAB::LINE, lhs);

    print("RHS:\n");
    print_expr(TAB::SPACE, rhs);

    m_tab_stack.pop_back();
}

void AST_Printer::print_literal(unsigned int indent, const Literal* literal)
{
    m_tab_stack.push_back(indent);

    switch (literal->type)
    {
        case LITERAL_INTEGER:
        {
            print("INTEGER: %llu\n", literal->data.integer_value);
            break;
        }
        case LITERAL_FLOAT:
        {
            print("FLOAT: %f\n", literal->data.float_value);
            break;
        }
        case LITERAL_CHAR:
        {
            print("CHAR: '%c'\n", literal->data.character);
            break;
        }
        case LITERAL_STRING:
        {
            print("STRING: \"");
            for (unsigned int i = 0; i < literal->data.string.len; i++)
            {
                char c = literal->data.string.ptr[i];
                if (IS_ALPHA_NUM(c) || (c == ' ')) {
                    putchar(c);
                } else {
                    printf("'0x%hhX'", c);
                }
            }
            printf("\"\n");
            break;
        }
    }

    m_tab_stack.pop_back();
}

void AST_Printer::print_func_call(unsigned int indent, const Expression::Func_Call* f_call)
{
    m_tab_stack.push_back(indent);

    print("FUNCTION:\n");
    print_expr(TAB::LINE, f_call->function);

    print("ARGUMENTS:\n");
    m_tab_stack.push_back(TAB::SPACE);
    for(List<Expression>::Element* it = f_call->arguments.head; it != nullptr; it = it->next)
    {
        print("ARG\n");
        print_expr((it->next == nullptr) ? TAB::SPACE : TAB::LINE, it->ptr);
    }

    m_tab_stack.pop_back();
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
        print("IDENTIFIER: %.*s\n", id->len, id->ptr);
    }
    else
    {
        print("IDENTIFIER: NULL\n");
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
            print("TYPE:\n");
            print_identifier(TAB::SPACE, &type->data.identifier);
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
    print_body(TAB::SPACE, &func->body);

    m_tab_stack.pop_back();
}

void AST_Printer::print_variable_def(unsigned int indent, const Statement::Variable* var)
{
    m_tab_stack.push_back(indent);

    print("NAME: %s\n", (var->name.len > 0) ? var->name.ptr : "NULL");

    print("DATATYPE:\n");
    print_type(TAB::LINE, var->type);

    if(var->value == nullptr)
    {
        print("VALUE: NULL\n");
    }
    else
    {
        print("VALUE:\n");
        print_expr(TAB::SPACE, var->value);
    }

    m_tab_stack.pop_back();
}

void AST_Printer::Print(const AST* ast)
{
    AST_Printer printer;

    printf("AST:\n");
    for (List<Statement>::Element* it = ast->statements.head; it != nullptr; it = it->next)
    {
        printer.print_statement((it->next == nullptr) ? TAB::SPACE : TAB::LINE, it->ptr);
    }
}

#include <debug.hpp>

#include <stdio.h>

void debug_indent(unsigned int level)
{
    for(unsigned int i = 0; i < level * 4; i++) printf(" ");
}

void debug_print_expr(Expression* expr, unsigned int level = 0)
{
    if(expr == nullptr) return;

    debug_indent(level);

    switch(expr->type)
    {
        case EXPR_OPERATION:
        {
            switch(expr->operation.op)
            {
                case EXPR_OP_ADD:                    { printf("+\n");   break; }
                case EXPR_OP_SUB:                    { printf("-\n");   break; }
                case EXPR_OP_MUL:                    { printf("*\n");   break; }
                case EXPR_OP_DIV:                    { printf("/\n");   break; }
                case EXPR_OP_LOGICAL_NOT:            { printf("NOT\n"); break; }
                case EXPR_OP_LOGICAL_AND:            { printf("AND\n"); break; }
                case EXPR_OP_LOGICAL_OR:             { printf("OR\n");  break; }
                case EXPR_OP_BITWISE_COMPLEMENT:     { printf("~\n");   break; }
                case EXPR_OP_BITWISE_XOR:            { printf("^\n");   break; }
                case EXPR_OP_BITWISE_AND:            { printf("&\n");   break; }
                case EXPR_OP_BITWISE_OR:             { printf("|\n");   break; }
                case EXPR_OP_BITWISE_L_SHIFT:        { printf("<<\n");  break; }
                case EXPR_OP_BITWISE_R_SHIFT:        { printf(">>\n");  break; }
                case EXPR_OP_CMP_EQUAL:              { printf("==\n");  break; }
                case EXPR_OP_CMP_NOT_EQUAL:          { printf("!=\n");  break; }
                case EXPR_OP_CMP_LESS_THAN:          { printf("<\n");   break; }
                case EXPR_OP_CMP_MORE_THAN:          { printf(">\n");   break; }
                case EXPR_OP_CMP_LESS_THAN_OR_EQUAL: { printf("<=\n");  break; }
                case EXPR_OP_CMP_MORE_THAN_OR_EQUAL: { printf(">=\n");  break; }
                case EXPR_OP_REFERENCE:              { printf("&\n");   break; }
                case EXPR_OP_DEREFERENCE:            { printf("*\n");   break; }
                case EXPR_OP_ASSIGN:                 { printf("=\n");   break; }
                case EXPR_OP_ARROW:                  { printf("->\n");  break; }
                default:                             { printf("unknown operator\n"); break; }
            }

            debug_print_expr(expr->operation.lhs, level + 1);
            debug_print_expr(expr->operation.rhs, level + 1);
            break;
        }

        case EXPR_LITERAL:
        {
            switch(expr->literal.type)
            {
                case LITERAL_INTEGER: { printf("%llu\n", expr->literal.integer.value); break; }
                case LITERAL_DECIMAL: { printf("%f\n",   expr->literal.decimal.value); break; }
                case LITERAL_STRING:  { printf("%.*s\n", expr->literal.string.len, expr->literal.string.ptr); break; }
                default:              { printf("Unknown token\n"); break; }
            }
            break;
        }

        case EXPR_IDENTIFIER:
        {
            printf("%.*s\n", expr->identifier.name.len, expr->identifier.name.ptr);
            break;
        }

        case EXPR_CALL:
        {
            printf("CALL:\n");

            debug_indent(level + 1);
            printf("FUNC_NAME: %.*s\n", expr->call.func_name.len, expr->call.func_name.ptr);

            for(Argument* a = expr->call.arguments; a != nullptr; a = a->next)
            {
                debug_indent(level + 1);
                printf("ARG:\n");

                debug_print_expr(a->value, level + 2);
            }

            break;
        }

        default:
        {
            printf("Unknown expression\n");
            break;
        }
    }
}

void debug_print_type(uint8_t type, unsigned int level = 0)
{
    debug_indent(level);

    printf("TYPE: ");
    switch(type)
    {
        case TYPE_U8:  { printf("U8\n");         break; }
        case TYPE_U32: { printf("U32\n");        break; }
        default:       { printf("%hhu\n", type); break; }
    }
}

void debug_print_stmt(Statement* stmt, unsigned int level = 0)
{
    debug_indent(level);

    switch(stmt->type)
    {
        case STMT_BLOCK:
        {
            printf("BLOCK_START\n");
            break;
        }
        case STMT_DECL_VAR:
        {
            printf("VAR\n");
            debug_print_type(stmt->decl_var.type, level + 1);
            
            debug_indent(level + 1);
            printf("NAME: %.*s\n", stmt->decl_var.name.len, stmt->decl_var.name.ptr);
            
            debug_indent(level + 1);
            printf("EXPR:\n");
            debug_print_expr(stmt->decl_var.value, level + 2);

            break;
        }

        case STMT_RET:
        {
            printf("RETURN\n");
            debug_print_expr(stmt->ret_stmt.expression, level + 1);
            break;
        }

        case STMT_EXPR:
        {
            printf("EXPR\n");
            debug_print_expr(stmt->expr, level + 1);
            break;
        }

        case STMT_IF:
        {
            printf("IF\n");
            
            debug_indent(level + 1);
            printf("COND:\n");
            debug_print_expr(stmt->if_stmt.condition, level + 2);

            debug_indent(level + 1);
            printf("BODY:\n");
            for (Statement* s = stmt->if_stmt.body; s != nullptr; s = s->next)
            {
                debug_print_stmt(s, level + 2);
            }

            break;
        }

        default:
        {
            printf("Unknown statement\n");
            break;
        }
    }
}

void debug_print_func(Function* func)
{
    printf("FUNC:\n");

    debug_print_type(func->ret_type, 1);

    debug_indent(1);
    printf("NAME: %.*s\n", func->name.len, func->name.ptr);

    for(Parameter* p = func->params; p != nullptr; p = p->next)
    {
        debug_indent(1);
        printf("PARAMETER:\n");

        debug_print_type(p->type, 2);

        debug_indent(2);   
        printf("NAME: %.*s\n", p->name.len, p->name.ptr);
    }

    debug_indent(1);
    printf("BODY:\n");
    for(Statement* s = func->body; s != nullptr; s = s->next)
    {
        debug_print_stmt(s, 2);
    }
    
}

void debug_print_ast(Root* ast)
{
    for(Function* f = ast->functions; f != nullptr; f = f->next)
    {
        debug_print_func(f);
    }
}
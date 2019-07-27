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
                case EXPR_OP_REFERENCE:              { printf("REF\n");   break; }
                case EXPR_OP_DEREFERENCE:            { printf("DEREF\n");   break; }
                case EXPR_OP_ASSIGN:                 { printf("=\n");   break; }
                case EXPR_OP_ARROW:                  { printf("->\n");  break; }
                default:                             { printf("Unknown operator\n"); break; }
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
                case LITERAL_STRING:  { printf("\"%.*s\"\n", expr->literal.string.len, expr->literal.string.ptr); break; }
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

void debug_print_type(DataType type, unsigned int level = 0)
{
    debug_indent(level);

    printf("TYPE: ");

    if(type.flags.is_external_symbol) printf("EXTERN ");
    if(type.flags.is_constant)        printf("CONST ");

    switch(type.type)
    {
        case TYPE_VOID: { printf("void");            break; }
        case TYPE_U8:   { printf("U8");              break; }
        case TYPE_U32:  { printf("U32");             break; }
        default:        { printf("%hhu", type.type); break; }
    }

    if(type.flags.is_pointer)          printf("* ");
    if(type.flags.is_fixed_size_array)
    {
        for(FixedSizeArray* it = type.fixed_size_array; it != nullptr; it = it->next)
        {
            printf("[%u]", it->size);
        }
    }

    printf("\n");
}

void debug_print_stmt(Statement* stmt, unsigned int level = 0)
{
    debug_indent(level);

    switch(stmt->type)
    {
        case STMT_FUNCTION_DEF:
        case STMT_FUNCTION_DECL:
        {
            printf("%s\n", stmt->type == STMT_FUNCTION_DEF ? "FUNCTION DEFINITION" : "FUNCTION DECLARATION");

            debug_print_type(stmt->function.ret_type, level + 1);

            debug_indent(level + 1);
            printf("NAME: %.*s\n", stmt->function.name.len, stmt->function.name.ptr);

            for(Parameter* p = stmt->function.params; p != nullptr; p = p->next)
            {
                debug_indent(level + 1);
                printf("PARAMETER:\n");

                debug_print_type(p->type, level + 2);

                debug_indent(level + 2);   
                printf("NAME: %.*s\n", p->name.len, p->name.ptr);
            }

            if(stmt->type == STMT_FUNCTION_DEF)
            {
                debug_indent(level + 1);
                printf("BODY:\n");
                for(Statement* s = stmt->function.body; s != nullptr; s = s->next)
                {
                    debug_print_stmt(s, level + 2);
                }
            }

            break;
        }

        case STMT_STRUCT:
        {
            printf("STRUCT\n");

            debug_indent(level + 1);
            printf("NAME: %.*s\n", stmt->struct_def.name.len, stmt->struct_def.name.ptr);

            debug_indent(level + 1);
            printf("MEMBERS\n");

            for(Statement* s = stmt->struct_def.members; s != nullptr; s = s->next)
            {
                debug_print_stmt(s, level + 2);
            }

            break;
        }

        case STMT_VARIABLE:
        {
            printf("VAR\n");
            debug_print_type(stmt->variable.type, level + 1);
            
            debug_indent(level + 1);
            printf("NAME: %.*s\n", stmt->variable.name.len, stmt->variable.name.ptr);
            
            debug_indent(level + 1);
            printf("EXPR:\n");
            debug_print_expr(stmt->variable.value, level + 2);

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

void debug_print_ast(AST* ast)
{
    for(Statement* stmt = ast->statements; stmt != nullptr; stmt = stmt->next)
    {
        debug_print_stmt(stmt);
    }
}

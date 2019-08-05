#include <debug.hpp>

#include <stdio.h>

void debug_indent(unsigned int level)
{
    for(unsigned int i = 0; i < level * 4; i++) printf(" ");
}

void debug_print_expr(Expression* expr, unsigned int level)
{
    if(expr == nullptr) return;

    debug_indent(level);

    switch(expr->type)
    {
        case EXPR_SUB_EXPR:
        {
            printf("SUB_EXPR:\n");
            debug_print_expr(expr->sub_expr, level + 1);
            break;
        }

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
                case EXPR_OP_REFERENCE:              { printf("REF\n");    break; }
                case EXPR_OP_DEREFERENCE:            { printf("DEREF\n");  break; }
                case EXPR_OP_ASSIGN:                 { printf("=\n");      break; }
                case EXPR_OP_ACCESS_FIELD:           { printf("ACCESS\n"); break; }
                case EXPR_OP_ARROW:                  { printf("->\n");     break; }
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
            printf("FUNCTION:\n");

            debug_print_expr(expr->call.function, level + 2);

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

void debug_print_struct(const Structure* structure, unsigned level)
{
    debug_indent(level);
    printf("STRUCT %.*s:\n", structure->name.len, structure->name.ptr);

    for(Structure::Member* m = structure->members; m != nullptr; m = m->next)
    {
        debug_indent(level + 1);
        printf("VARIABLE %.*s:\n", m->name.len, m->name.ptr);
        debug_print_variable(m->variable, level + 2);
    }
}

void debug_print_variable(const Variable* var, unsigned int level)
{   
    debug_indent(level);
    
    if(var->flags.value != 0)
    {
        if(var->flags.is_external_symbol)
        {
            printf("EXTERN ");
        }

        if(var->flags.is_constant)
        {
            printf("CONST ");
        }
    }

    if(var->type == TYPE_STRUCT)
    {
        printf("STRUCT:\n");
        debug_print_struct(var->structure, level + 1);
    }
    else
    {
        const char* type_str = "UNKNOWN";
        switch(var->type)
        {
            case TYPE_U8:     { type_str = "U8";     break; }
            case TYPE_U16:    { type_str = "U16";    break; }
            case TYPE_U32:    { type_str = "U32";    break; }
            case TYPE_U64:    { type_str = "U64";    break; }
            case TYPE_I8:     { type_str = "I8";     break; }
            case TYPE_I16:    { type_str = "I16";    break; }
            case TYPE_I32:    { type_str = "I32";    break; }
            case TYPE_I64:    { type_str = "I64";    break; }
            case TYPE_F32:    { type_str = "I64";    break; }
            case TYPE_F64:    { type_str = "I64";    break; }
            case TYPE_VOID:   { type_str = "VOID";   break; }
            case TYPE_PTR:    { type_str = "PTR";    break; }
            case TYPE_ARRAY:  { type_str = "ARRAY";  break; }
            case TYPE_STRUCT: { type_str = "STRUCT"; break; }
            default:          { break; }
        }

        printf("%s\n", type_str);

        switch(var->type)
        {
            case TYPE_PTR:
            {
                debug_print_variable(var->pointer, level + 1);
                break;
            }

            case TYPE_ARRAY:
            {
                debug_indent(level + 1);
                printf("SIZE: %u\n", var->array.size);
                
                debug_indent(level + 1);
                printf("ELEMENTS:\n");
                debug_print_variable(var->array.elements, level + 2);
                break;
            }

            case TYPE_STRUCT:
            {
                for(Structure::Member* it = var->structure->members; it != nullptr; it = it->next)
                {
                    debug_print_variable(it->variable, level + 1);
                }
                break;
            }
        }
    }
}

void debug_print_struct_member(Structure::Member* member, unsigned int level)
{
    debug_indent(level);
    printf("MEMBER: %.*s\n", member->name.len, member->name.ptr);

    debug_print_variable(member->variable, level + 1);
}

void debug_print_stmt(Statement* stmt, unsigned int level)
{
    debug_indent(level);

    switch(stmt->type)
    {
        case STMT_FUNCTION_DEF:
        case STMT_FUNCTION_DECL:
        {
            printf("%s\n", stmt->type == STMT_FUNCTION_DEF ? "FUNCTION DEFINITION" : "FUNCTION DECLARATION");

            debug_print_variable(stmt->function.ptr->ret_type, level + 1);

            debug_indent(level + 1);
            printf("NAME: %.*s\n", stmt->function.name.len, stmt->function.name.ptr);

            for(Parameter* p = stmt->function.ptr->params; p != nullptr; p = p->next)
            {
                debug_indent(level + 1);
                printf("PARAMETER:\n");

                debug_print_variable(p->type, level + 2);

                debug_indent(level + 2);   
                printf("NAME: %.*s\n", p->name.len, p->name.ptr);
            }

            if(stmt->type == STMT_FUNCTION_DEF)
            {
                debug_indent(level + 1);
                printf("BODY:\n");
                for(Statement* s = stmt->function.ptr->body; s != nullptr; s = s->next)
                {
                    debug_print_stmt(s, level + 2);
                }
            }

            break;
        }

        case STMT_STRUCT_DEF:
        {
            printf("STRUCT\n");

            debug_indent(level + 1);
            printf("NAME: %.*s\n", stmt->struct_def.name.len, stmt->struct_def.name.ptr);

            debug_indent(level + 1);
            printf("MEMBERS\n");

            for(Structure::Member* m = stmt->struct_def.structure->members; m != nullptr; m = m->next)
            {
                debug_print_struct_member(m, level + 2);
            }

            break;
        }

        case STMT_VARIABLE_DECL:
        {
            printf("VAR\n");
            debug_print_variable(stmt->variable.type, level + 1);
            
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
        printf("\n");
    }
}

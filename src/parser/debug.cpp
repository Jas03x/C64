#include <debug.hpp>

#include <stdio.h>

void debug_indent(unsigned int level)
{
    for(unsigned int i = 0; i < level * 4; i++) printf(" ");
}

void debug_print_identifier(Identifier* id)
{
    Identifier* it = id;
    while(true)
    {
        printf("%.*s", it->str.len, it->str.ptr);

        it = it->next;
        if(it != nullptr)
        {
            printf("::");
        }
        else
        {
            printf("\n");
            break;
        }
    }
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

		case EXPR_STATIC_CAST:
		{
			printf("STATIC CAST:\n");
			goto PRINT_CAST;
		}

		case EXPR_REINTERPRET_CAST:
		{
			printf("REINTERPRET CAST:\n");
			goto PRINT_CAST;
		}

		PRINT_CAST:
		{
			debug_indent(level + 1);
			printf("VARIABLE:\n");
			debug_print_variable(expr->cast.type, level + 2);

			debug_indent(level + 1);
			printf("EXPR:\n");
			debug_print_expr(expr->cast.expr, level + 2);
			break;
		}

		case EXPR_FUNCTION_CALL:
		{
			printf("CALL:\n");

			debug_indent(level + 1);
			printf("FUNCTION:\n");

			debug_print_expr(expr->call.function, level + 2);

			for (Argument* a = expr->call.arguments; a != nullptr; a = a->next)
			{
				debug_indent(level + 1);
				printf("ARG:\n");

				debug_print_expr(a->value, level + 2);
			}

			break;
		}

        case EXPR_OPERATION:
        {
			switch (expr->operation.op)
			{
				case EXPR_OP_ADD: { printf("+\n");   break; }
				case EXPR_OP_SUB: { printf("-\n");   break; }
				case EXPR_OP_MUL: { printf("*\n");   break; }
				case EXPR_OP_DIV: { printf("/\n");   break; }
				case EXPR_OP_LOGICAL_NOT: { printf("NOT\n"); break; }
				case EXPR_OP_LOGICAL_AND: { printf("AND\n"); break; }
				case EXPR_OP_LOGICAL_OR: { printf("OR\n");  break; }
				case EXPR_OP_BITWISE_COMPLEMENT: { printf("~\n");   break; }
				case EXPR_OP_BITWISE_XOR: { printf("^\n");   break; }
				case EXPR_OP_BITWISE_AND: { printf("&\n");   break; }
				case EXPR_OP_BITWISE_OR: { printf("|\n");   break; }
				case EXPR_OP_BITWISE_L_SHIFT: { printf("<<\n");  break; }
				case EXPR_OP_BITWISE_R_SHIFT: { printf(">>\n");  break; }
				case EXPR_OP_CMP_EQUAL: { printf("==\n");  break; }
				case EXPR_OP_CMP_NOT_EQUAL: { printf("!=\n");  break; }
				case EXPR_OP_CMP_LESS_THAN: { printf("<\n");   break; }
				case EXPR_OP_CMP_MORE_THAN: { printf(">\n");   break; }
				case EXPR_OP_CMP_LESS_THAN_OR_EQUAL: { printf("<=\n");  break; }
				case EXPR_OP_CMP_MORE_THAN_OR_EQUAL: { printf(">=\n");  break; }
				case EXPR_OP_ASSIGN: { printf("=\n");   break; }
				case EXPR_OP_ARROW: { printf("->\n");  break; }
				case EXPR_OP_REFERENCE: { printf("REF\n");    break; }
				case EXPR_OP_DEREFERENCE: { printf("DEREF\n");  break; }
				case EXPR_OP_ACCESS_FIELD: { printf("ACCESS\n"); break; }
				case EXPR_OP_INDEX: { printf("INDEX\n");  break; }
				default: { printf("Unknown operator\n"); break; }
			}

			debug_print_expr(expr->operation.lhs, level + 1);
			debug_print_expr(expr->operation.rhs, level + 1);
			break;
        }

        case EXPR_INITIALIZER:
        {
            printf("INITIALIZER:\n");
            if(expr->initializer.values == nullptr)
            {
                debug_indent(level + 1);
                printf("NULL\n");
            }
            else
            {
                for(Initializer::Value* value = expr->initializer.values; value != nullptr; value = value->next)
                {
                    debug_indent(level + 1);
                    printf("VALUE:\n");
                    debug_print_expr(value->expr, level + 2);
                }
            }
            break;
        }

        case EXPR_LITERAL:
        {
            switch(expr->literal.type)
            {
                case LITERAL_INTEGER: { printf("%llu\n", expr->literal.integer.value); break; }
                case LITERAL_DECIMAL: { printf("%f\n",   expr->literal.decimal.value); break; }
                case LITERAL_CHAR:    { printf("'%c'\n",   expr->literal.character);     break; }
                case LITERAL_STRING:  { printf("\"%.*s\"\n", expr->literal.string.len, expr->literal.string.ptr); break; }
                default:              { printf("Unknown token\n"); break; }
            }
            break;
        }

        case EXPR_IDENTIFIER:
        {
            debug_print_identifier(expr->identifier);
            break;
        }

        default:
        {
            printf("Unknown expression\n");
            break;
        }
    }
}

void debug_print_composite(const Composite* composite, unsigned level)
{
    debug_indent(level);
    
	const char* type = "INVALID";
	switch (composite->type)
	{
		case COMP_TYPE_STRUCT: { type = "STRUCT"; break; }
		case COMP_TYPE_UNION:  { type = "UNION";  break; }
		default: { break; }
	}

    if(composite->name.len == 0)
    {
        printf("%s:\n", type);
    }
    else
    {
        printf("%s: %.*s\n", type, composite->name.len, composite->name.ptr);
    }

    for(Statement* stmt = composite->body; stmt != nullptr; stmt = stmt->next)
    {
        debug_indent(level + 1);
		debug_print_stmt(stmt, level + 1);
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

    if(var->type == TYPE_UNKNOWN)
    {
        if(var->identifier != nullptr)
        {
            printf("TYPE: ");
            debug_print_identifier(var->identifier);
        }
    }
    else if(var->type == TYPE_COMPOSITE)
    {
        if(var->composite->name.len > 0)
        {
            printf("%s: %.*s\n", var->composite->type == COMP_TYPE_UNION ? "UNION" : "STRUCT", var->composite->name.len, var->composite->name.ptr);
        }
        else
        {
            printf("UNNAMED STRUCT:\n");
            debug_print_composite(var->composite, level + 1);
        }
    }
    else if(var->type == TYPE_ENUM)
    {
        debug_print_enum(var->enumerator, level);
    }
    else
    {
        const char* type_str = "UNKNOWN";
        switch(var->type)
        {
            case TYPE_U8:        { type_str = "U8";     break; }
            case TYPE_U16:       { type_str = "U16";    break; }
            case TYPE_U32:       { type_str = "U32";    break; }
            case TYPE_U64:       { type_str = "U64";    break; }
            case TYPE_I8:        { type_str = "I8";     break; }
            case TYPE_I16:       { type_str = "I16";    break; }
            case TYPE_I32:       { type_str = "I32";    break; }
            case TYPE_I64:       { type_str = "I64";    break; }
            case TYPE_F32:       { type_str = "I64";    break; }
            case TYPE_F64:       { type_str = "I64";    break; }
            case TYPE_VOID:      { type_str = "VOID";   break; }
            case TYPE_PTR:       { type_str = "PTR";    break; }
            case TYPE_COMPOSITE: { type_str = "STRUCT"; break; }
			case TYPE_FUNCTION_POINTER:     { type_str = "FUNCTION POINTER";     break; }
            case TYPE_CONSTANT_SIZED_ARRAY: { type_str = "CONSTANT SIZED ARRAY"; break; }
            case TYPE_VARIABLE_SIZED_ARRAY: { type_str = "VARIABLE SIZED ARRAY"; break; }
            default: { break; }
        }

        printf("%s\n", type_str);

        switch(var->type)
        {
            case TYPE_PTR:
            {
                debug_print_variable(var->pointer, level + 1);
                break;
            }

			case TYPE_FUNCTION_POINTER:
			{
				debug_indent(level + 1);
				printf("RETURN:\n");
				debug_print_variable(var->func_ptr.ret_type, level + 2);

				debug_indent(level + 1);
				printf("PARAMETERS:\n");
				for (Parameter* p = var->func_ptr.parameters; p != nullptr; p = p->next)
				{
					debug_indent(level + 1);
					printf("PARAMETER:\n");

					debug_print_variable(p->type, level + 2);

					debug_indent(level + 2);
					printf("NAME: %.*s\n", p->name.len, p->name.ptr);
				}
				break;
			}

            case TYPE_CONSTANT_SIZED_ARRAY:
            case TYPE_VARIABLE_SIZED_ARRAY:
            {
                debug_indent(level + 1);
                printf("SIZE:\n");
                if(var->array.size != nullptr) { debug_print_expr(var->array.size, level + 2); }
                else                           { debug_indent(level + 2); printf("NULL\n"); }
                
                debug_indent(level + 1);
                printf("ELEMENTS:\n");
                debug_print_variable(var->array.elements, level + 2);
                break;
            }

            case TYPE_COMPOSITE:
            {
                for(Statement* it = var->composite->body; it != nullptr; it = it->next)
                {
                    debug_print_stmt(it, level + 1);
                }
                break;
            }
        }
    }
}

void debug_print_stmt(Statement* stmt, unsigned int level)
{
    debug_indent(level);

    switch(stmt->type)
    {
        case STMT_IMPORT:
        {
            printf("IMPORT: %.*s\n", stmt->import_stmt.module_name.len, stmt->import_stmt.module_name.ptr);
            break;
        }

        case STMT_EXPORT:
        {
            printf("EXPORT: %.*s\n", stmt->export_stmt.module_name.len, stmt->export_stmt.module_name.ptr);
            break;
        }

        case STMT_MODULE:
        {
            printf("%s MODULE: %.*s\n", stmt->module_decl.access == ACCESS::PUBLIC ? "PUBLIC" : "PRIVATE", stmt->module_decl.name.len, stmt->module_decl.name.ptr);
            break;
        }

        case STMT_ENUM_DEF:
        {
            debug_print_enum(stmt->enum_def.enumerator);
            break;
        }

        case STMT_BREAK:
        {
            printf("BREAK\n");
            break;
        }

        case STMT_SWITCH:
        {
            debug_print_switch(stmt, level);
            break;
        }

        case STMT_DEFAULT_CASE:
        {
            printf("DEFAULT:\n");
            break;
        }

        case STMT_CASE:
        {
            debug_print_case(stmt, level);
            break;
        }

        case STMT_CONTINUE:
        {
            printf("CONTINUE\n");
            break;
        }

        case STMT_LABEL:
        {
            printf("LABEL: %.*s\n", stmt->label.name.len, stmt->label.name.ptr);
            break;
        }

        case STMT_GOTO:
        {
            printf("GOTO: %.*s\n", stmt->goto_stmt.target.len, stmt->goto_stmt.target.ptr);
            break;
        }

        case STMT_COMPOUND_STMT:
        {
            printf("COMPOUND STMT:\n");
            for(Statement* s = stmt->compound_stmt.body; s != nullptr; s = s->next)
            {
                debug_print_stmt(s, level + 1);
            }
            break;
        }

        case STMT_TYPEDEF:
        {
            debug_print_typedef(stmt, level);
            break;
        }

        case STMT_NAMESPACE:
        {
            debug_print_namespace(stmt, level);
            break;
        }

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

        case STMT_COMP_DEF:
        {
			printf("COMPOSITE:\n");
			debug_print_composite(stmt->comp_def.composite, level + 1);
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
        case STMT_ELSE:
        case STMT_ELSE_IF:
        {
            const char* output = "IF";
            switch(stmt->type)
            {
                case STMT_ELSE:    { output = "ELSE";    break; }
                case STMT_ELSE_IF: { output = "ELSE_IF"; break; }
                default: break;
            }
            printf("%s\n", output);
            
            if(stmt->type != STMT_ELSE)
            {
                debug_indent(level + 1);
                printf("COND:\n");
                debug_print_expr(stmt->if_stmt.condition, level + 2);
            }

            Statement* body = (stmt->type == STMT_ELSE) ? stmt->else_stmt.body : stmt->if_stmt.body;

            debug_indent(level + 1);
            printf("BODY:\n");
            for (Statement* s = body; s != nullptr; s = s->next)
            {
                debug_print_stmt(s, level + 2);
            }

            break;
        }

        case STMT_WHILE:
        {
            printf("WHILE\n");

            debug_indent(level + 1);
            printf("CONDITION:\n");
            debug_print_expr(stmt->while_stmt.condition, level + 2);

            debug_indent(level + 1);
            printf("BODY:\n");
            for (Statement* s = stmt->while_stmt.body; s != nullptr; s = s->next)
            {
                debug_print_stmt(s, level + 2);
            }

            break;
        }

        case STMT_FOR:
        {
            printf("FOR\n");

            debug_indent(level + 1);
            printf("VARIABLE:\n");
            if(stmt->for_stmt.variable != nullptr) { debug_print_stmt(stmt->for_stmt.variable, level + 2); }
            
            debug_indent(level + 1);
            printf("CONDITION:\n");
            if(stmt->for_stmt.condition != nullptr) debug_print_expr(stmt->for_stmt.condition, level + 2);

            debug_indent(level + 1);
            printf("STEP:\n");
            if(stmt->for_stmt.step) { debug_print_expr(stmt->for_stmt.step, level + 2); }

            debug_indent(level + 1);
            printf("BODY:\n");
            for (Statement* s = stmt->for_stmt.body; s != nullptr; s = s->next)
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

void debug_print_token(const Token& tk)
{
    const char* str = "UNKNOWN";
    switch(tk.type)
    {
        case TK_INVALID: { str = "TK_INVALID"; break; }
        case TK_CONST: { str = "TK_CONST"; break; }
        case TK_EXTERN: { str = "TK_EXTERN"; break; }
        case TK_STRUCT: { str = "TK_STRUCT"; break; }
        case TK_RETURN: { str = "TK_RETURN"; break; }
        case TK_IF: { str = "TK_IF"; break; }
        case TK_EQUAL: { str = "TK_EQUAL"; break; }
        case TK_LEFT_ARROW_HEAD: { str = "TK_LEFT_ARROW_HEAD"; break; }
        case TK_RIGHT_ARROW_HEAD: { str = "TK_RIGHT_ARROW_HEAD"; break; }
        case TK_PLUS: { str = "TK_PLUS"; break; }
        case TK_MINUS: { str = "TK_MINUS"; break; }
        case TK_DOT: { str = "TK_DOT"; break; }
        case TK_ASTERISK: { str = "TK_ASTERISK"; break; }
        case TK_FORWARD_SLASH: { str = "TK_FORWARD_SLASH"; break; }
        case TK_OPEN_CURLY_BRACKET: { str = "TK_OPEN_CURLY_BRACKET"; break; }
        case TK_CLOSE_CURLY_BRACKET: { str = "TK_CLOSE_CURLY_BRACKET"; break; }
        case TK_OPEN_ROUND_BRACKET: { str = "TK_OPEN_ROUND_BRACKET"; break; }
        case TK_CLOSE_ROUND_BRACKET: { str = "TK_CLOSE_ROUND_BRACKET"; break; }
        case TK_OPEN_SQUARE_BRACKET: { str = "TK_OPEN_SQUARE_BRACKET"; break; }
        case TK_CLOSE_SQUARE_BRACKET: { str = "TK_CLOSE_SQUARE_BRACKET"; break; }
        case TK_SEMICOLON: { str = "TK_SEMICOLON"; break; }
        case TK_LITERAL: { str = "TK_LITERAL"; break; }
        case TK_IDENTIFIER: { str = "TK_IDENTIFIER"; break; }
        case TK_COMMA: { str = "TK_COMMA"; break; }
        case TK_OR: { str = "TK_OR"; break; }
        case TK_AND: { str = "TK_AND"; break; }
        case TK_CARET: { str = "TK_CARET"; break; }
        case TK_EXPLANATION_MARK: { str = "TK_EXPLANATION_MARK"; break; }
        case TK_AMPERSAND: { str = "TK_AMPERSAND"; break; }
        case TK_PERCENT: { str = "TK_PERCENT"; break; }
        case TK_NAMESPACE: { str = "TK_NAMESPACE"; break; }
        case TK_TYPE: { str = "TK_TYPE"; break; }
        case TK_FOR: { str = "TK_FOR"; break; }
        case TK_WHILE: { str = "TK_WHILE"; break; }
        case TK_EOF: { str = "TK_EOF"; break; }
		case TK_COLON: { str = "TK_COLON"; break; }
        case TK_UNION: { str = "UNION"; break; }
        default: break;
    }

    printf("%s ", str);

    switch(tk.type)
    {
        case TK_IDENTIFIER:
        {
            printf("- %.*s\n", tk.identifier.string.len, tk.identifier.string.ptr);
            break;
        }

        case TK_TYPE:
        {
            switch(tk.subtype)
            {
                case TK_TYPE_INVALID: { printf("INVALID\n"); break; }
                case TK_TYPE_VOID: { printf("VOID\n"); break; }
                case TK_TYPE_U8: { printf("U8\n"); break; }
                case TK_TYPE_U16: { printf("U16\n"); break; }
                case TK_TYPE_U32: { printf("U32\n"); break; }
                case TK_TYPE_U64: { printf("U64\n"); break; }
                case TK_TYPE_I8: { printf("I8\n"); break; }
                case TK_TYPE_I16: { printf("I16\n"); break; }
                case TK_TYPE_I32: { printf("I32\n"); break; }
                case TK_TYPE_I64: { printf("I64\n"); break; }
                case TK_TYPE_F32: { printf("F32\n"); break; }
                case TK_TYPE_F64: { printf("F64\n"); break; }
                default: break;
            }

            break;
        }

        case TK_LITERAL:
        {
            switch(tk.literal.type)
            {
                case LITERAL_DECIMAL:
                {
                    printf("- decimal (%f)\n", tk.literal.decimal.value);
                    break;
                }

                case LITERAL_INTEGER:
                {
                    printf("- integer (%llu)\n", tk.literal.integer.value);
                    break;
                }

                case LITERAL_STRING:
                {
                    printf("- string (\"%.*\")\n", tk.literal.string.len, tk.literal.string.ptr);
                    break;
                }

                case LITERAL_CHAR:
                {
                    printf("- character (%c)\n", tk.literal.character);
                    break;
                }

                default: break;
            }

            break;
        }

        default:
        {
            printf("\n");
            break;
        }
    }
}

void debug_print_enum(const Enum* enumerator, unsigned int level)
{
    debug_indent(level);
    printf("ENUM:");
    
    if(enumerator->name.len > 0)
    {
        printf(" %.*s\n", enumerator->name.len, enumerator->name.ptr);
    }
    else
    {
        printf("\n");
    }

    for(Enum::Value* v = enumerator->values; v != nullptr; v = v->next)
    {
        printf("VALUE %.*s = ", v->name.len, v->name.ptr);
        switch(v->value.type)
        {
            case LITERAL_INTEGER: { printf("%llu\n", v->value.integer.value); break; }
            case LITERAL_DECIMAL: { printf("%f\n",   v->value.decimal.value); break; }
            case LITERAL_CHAR:    { printf("'%c'\n",   v->value.character);     break; }
            case LITERAL_STRING:  { printf("\"%.*s\"\n", v->value.string.len, v->value.string.ptr); break; }
            default:              { printf("Unknown token\n"); break; }
        }
    }
}

void debug_print_switch(const Statement* statement, unsigned int level)
{
    debug_indent(level);
    printf("SWITCH:\n");

    debug_indent(level + 1);
    printf("EXPR:\n");
    debug_print_expr(statement->switch_stmt.expr, level + 2);

    debug_indent(level + 1);
    printf("BODY:\n");
    for(Statement* s = statement->switch_stmt.body; s != nullptr; s = s->next)
    {
        debug_print_stmt(s, level + 2);
    }
}

void debug_print_case(const Statement* statement, unsigned int level)
{
    debug_indent(level);
    printf("CASE: ");
    switch(statement->case_stmt.value.type)
    {
        case LITERAL_INTEGER: { printf("%llu\n", statement->case_stmt.value.integer.value); break; }
        case LITERAL_DECIMAL: { printf("%f\n",   statement->case_stmt.value.decimal.value); break; }
        case LITERAL_CHAR:    { printf("'%c'\n",   statement->case_stmt.value.character);     break; }
        case LITERAL_STRING:  { printf("\"%.*s\"\n", statement->case_stmt.value.string.len, statement->case_stmt.value.string.ptr); break; }
        default:              { printf("Unknown token\n"); break; }
    }
    
}

void debug_print_namespace(const Statement* statement, unsigned int level)
{
    debug_indent(level);
    printf("NAMESPACE:\n");

    debug_indent(level + 1);
    printf("NAME: %.*s\n", statement->name_space.name.len, statement->name_space.name.ptr);

    debug_indent(level + 1);
    printf("BODY:\n");
    for(Statement* stmt = statement->name_space.statements; stmt != nullptr; stmt = stmt->next)
    {
        debug_print_stmt(stmt, level + 1);
    }
}

void debug_print_typedef(const Statement* statement, unsigned int level)
{
    debug_indent(level);
    printf("TYPEDEF: \n");
    
    debug_indent(level + 1);
    printf("NAME: %.*s\n", statement->type_def.name.len, statement->type_def.name.ptr);

    debug_indent(level + 1);
    printf("VARIABLE:\n");
    debug_print_variable(statement->type_def.variable, level + 2);
}

#include <parser.hpp>

#define error(str, ...) printf("[%s]: " str, __FUNCTION__, __VA_ARGS__)

Parser::Parser(TokenStack* stack)
{
    m_stack = stack;
}

Parser::~Parser()
{
}

AST* Parser::Parse(TokenStack* stack)
{
    Parser parser(stack);

    AST* ast = nullptr;
    parser.parse(&ast);

    return ast;
}

bool Parser::parse(AST** ptr)
{
    bool status = true;
    Statement *head = nullptr, *tail = nullptr;

    while(status)
    {
        if(m_stack->peek(0).type == TK_EOF)
        {
            m_stack->pop();
            break;
        }
        else
        {
            Statement* stmt = nullptr;
            
            if(parse_statement(&stmt))
            {
                if(stmt != nullptr)
                {
                    if(tail == nullptr) { head = stmt;       }
                    else                { tail->next = stmt; }
                    tail = stmt;
                }
            }
            else
            {
                status = false;
            }
        }
    }

    if(status)
    {
        AST* ast = new AST();
        ast->statements = head;

        *ptr = ast;
    }

    return status;
}

bool Parser::parse_identifier(Identifier** identifier)
{
	bool status = true;

	Token tk = {};
	Identifier *head = nullptr, *tail = nullptr;
	
	while(status)
	{
		tk = m_stack->pop();
		if(tk.type == TK_IDENTIFIER)
		{
			Identifier* id = new Identifier();
			id->str = tk.identifier.string;
			
			if(tail == nullptr) { head = id;       }
			else                { tail->next = id; }
			tail = id;
			
			if(m_stack->peek(0).type == TK_COLON)
			{
                m_stack->pop();
				if(m_stack->pop().type != TK_COLON)
				{
					status = false;
					error("expected ':'\n");
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			status = false;
			error("expected identifier\n");
		}
	}
	
    if(status)
    {
        *identifier = head;
    }

	return status;
}

bool Parser::parse_body(Statement** ptr)
{
    bool status = true;
    Statement *head = nullptr, *tail = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_OPEN_CURLY_BRACKET)
    {
        error("expected '{'\n");
        status = false;
    }

    while(status)
    {
        Token tk = m_stack->peek(0);
        
        if(tk.type == TK_CLOSE_CURLY_BRACKET)
        {
            m_stack->pop();
            break;
        }
        else
        {
            Statement* stmt = nullptr;
            
            if(parse_statement(&stmt))
            {
                if(stmt != nullptr)
                {
                    if(tail == nullptr) { head = stmt; }
                    else                { tail->next = stmt; }
                    tail = stmt;
                }
            }
            else
            {
                status = false;
            }
        }
    }

    if(status)
    {
        *ptr = head;
    }

    return status;
}

bool Parser::parse_composite(Composite** ptr)
{
    bool status = true;

    strptr name = {};
    uint8_t type = COMP_TYPE_INVALID;

    Token tk = m_stack->pop();
    switch(tk.type)
    {
        case TK_STRUCT: { type = COMP_TYPE_STRUCT; break; }
        case TK_UNION:  { type = COMP_TYPE_UNION;  break; }
        default:
        {
            status = false;
            error("Expected 'struct' or 'union'\n");
        }
    }
    
    if(status)
    {
        if(m_stack->peek(0).type == TK_IDENTIFIER)
        {
            tk = m_stack->pop();
            name = tk.identifier.string;
        }
    }

    Statement* body = nullptr;

    if(status)
    {
        tk = m_stack->pop();

        if(tk.type == TK_OPEN_CURLY_BRACKET)
        {
            status = parse_body(&body);
        }
        else if(tk.type == TK_SEMICOLON)
        {
            error("TODO: PARSE STRUCT PROTOTYPES\n");
            status = false;
        }
        else
        {
            error("Unexpected token %hhu\n", tk.type);
            status = false;
        }
    }

    if(status)
    {
        Composite* composite = new Composite();
        composite->type = type;
        composite->name = name;
        composite->body = body;

        *ptr = composite;
    }

    return status;
}

bool Parser::parse_composite_definition(Statement** ptr)
{
    Token tk = {};
    Composite* composite = nullptr;
    bool status = parse_composite(&composite);

    if(status && (composite->name.len == 0))
    {
        status = false;
        error("struct name cannot be null\n");
    }

    if(status)
    {
        tk = m_stack->pop();
        if(tk.type != TK_SEMICOLON)
        {
            status = false;
            error("expected ';'\n");
        }
    }

    if(status)
    {
        Statement* statement = new Statement();
        statement->type = STMT_COMP_DEF;
        statement->comp_def.name = composite->name;
        statement->comp_def.composite = composite;

        *ptr = statement;
    }

    return status;
}

bool Parser::parse_enum_definition(Statement** ptr)
{
    bool status = true;

    uint8_t type = STMT_INVALID;
    strptr name = {};
    Enum* enumerator = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_ENUM)
    {
        status = false;
        error("expected 'enum'\n");
    }

    if(status)
    {
        if(m_stack->peek(0).type == TK_IDENTIFIER)
        {
            tk = m_stack->pop();
            name = tk.identifier.string;
        }
    }
    
    if(status)
    {
        switch(m_stack->peek(0).type)
        {
            case ';':
            {
                type = STMT_ENUM_DECL;
                break;
            }

            case '{':
            {
                type = STMT_ENUM_DEF;

                status = parse_enum_body(&enumerator->values);
                if(status)
                {
                    if(enumerator->name.len == 0)
                    {
                        status = false;
                        error("enum name cannot be null\n");
                    }
                }
                break;
            }

            default:
            {
                status = false;
                error("unexpected token\n");
            }
        }
    }

    if(status)
    {
        tk = m_stack->pop();
        if(tk.type != TK_SEMICOLON)
        {
            status = false;
            error("expected ';'\n");
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = type;
        stmt->enum_def.name = enumerator->name;
        stmt->enum_def.enumerator = enumerator;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_namespace(Statement** ptr)
{
    bool status = true;

    strptr name = { 0 };
    Statement*  body = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_NAMESPACE)
    {
        status = false;
        error("expected 'namespace'\n");
    }

    tk = m_stack->pop();
    if(tk.type != TK_IDENTIFIER)
    {
        name = tk.identifier.string;
    }
    else
    {
        status = false;
        error("expected identifier\n");
    }

    if(status)
    {
        status = parse_body(&body);
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_NAMESPACE;
        stmt->name_space.name       = name;
        stmt->name_space.statements = body;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_function_pointer(strptr& name, Variable** ptr, Variable* ret_type)
{
	bool status = true;

	Variable* var = nullptr;

	if (m_stack->pop().type != TK_OPEN_ROUND_BRACKET)
	{
		status = false;
		error("expected '('\n");
	}
    else if(m_stack->pop().type != TK_ASTERISK)
    {
        status = false;
        error("expected '*'\n");
    }

    if(status)
    {
        Token tk = m_stack->peek(0);
        if (tk.type == TK_IDENTIFIER)
        {
            m_stack->pop();
            name = tk.identifier.string;
        }

        var = new Variable();
        var->type = TYPE_FUNCTION_POINTER;
        var->func_ptr.ret_type = ret_type;

        Variable* var_ptr = var; // a copy because the parse array function will over-write the pointer

        if (m_stack->peek(0).type == TK_OPEN_SQUARE_BRACKET)
        {
            status = parse_array(&var);
        }
        
        if (status)
        {
            if (m_stack->pop().type == TK_CLOSE_ROUND_BRACKET)
            {
                status = parse_parameters(&var_ptr->func_ptr.parameters);
            }
            else
            {
                status = false;
                error("expected ')'\n");
            }
        }
    }

	if (status)
	{
		*ptr = var;
	}

	return status;
}

bool Parser::parse_typedef(Statement** ptr)
{
    bool status = true;

    strptr name = {};
    Variable* variable = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_TYPEDEF)
    {
        status = false;
        error("expected 'typedef'\n");
    }

    if(status)
    {
        status = parse_variable(&variable);
    }

    if(status)
    {
		tk = m_stack->peek(0);

		if (tk.type == TK_IDENTIFIER)
		{
            name = tk.identifier.string;
		}
		else if (tk.type == TK_OPEN_ROUND_BRACKET)
		{
			status = parse_function_pointer(name, &variable, variable);
		}
		else
		{
			status = false;
			error("unexpected token\n");
		}
    }

    if(status)
    {
        if(m_stack->peek(0).type == TK_OPEN_SQUARE_BRACKET)
        {
            status = parse_array(&variable);
        }
    }

    if(status)
    {
        tk = m_stack->pop();
        if(tk.type != TK_SEMICOLON)
        {
            status = false;
            error("expected ';'\n");
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_TYPEDEF;
        stmt->type_def.name     = name;
        stmt->type_def.variable = variable;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_compound_stmt(Statement** ptr)
{
    bool status = true;

    Statement* body = nullptr;

    if(status)
    {
        status = parse_body(&body);
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_COMPOUND_STMT;
        stmt->compound_stmt.body = body;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_enum_value(Enum::Value** ptr)
{
    bool status = true;

    strptr  name = { };
    Literal value = { };

    Token tk = m_stack->pop();
    if(tk.type == TK_IDENTIFIER)
    {
        name = tk.identifier.string;
    }
    else
    {
        status = false;
        error("expected identifier\n");
    }

    if(m_stack->peek(0).type == TK_EQUAL)
    {
        m_stack->pop();
        
        if(m_stack->pop().type == TK_LITERAL)
        {
            value = tk.literal;
        }
        else
        {
            status = false;
            error("expected literal\n");
        }
    }
    
    if(status)
    {
        Enum::Value* entry = new Enum::Value();
        entry->name  = name;
        entry->value = value;

        *ptr = entry;
    }

    return status;
}

bool Parser::parse_enum_body(Enum::Value** ptr)
{
    bool status = true;

    Token tk = m_stack->pop();
    if(status)
    {
        if(m_stack->pop().type != TK_OPEN_CURLY_BRACKET)
        {
            status = false;
            error("expected '{'\n");
        }
    }

    Enum::Value *list_head = nullptr, *list_tail = nullptr;

    if(status)
    {
        if(m_stack->peek(0).type == TK_CLOSE_CURLY_BRACKET)
        {
            m_stack->pop();
        }
        else
        {
            while(status)
            {
                Enum::Value* entry = nullptr;
                status = parse_enum_value(&entry);

                if(status)
                {
                    if(list_tail == nullptr) { list_head = entry;       }
                    else                     { list_tail->next = entry; }
                    list_tail = entry;

                    tk = m_stack->pop();
                    if(tk.type == TK_CLOSE_CURLY_BRACKET)
                    {
                        break;
                    }
                    else if(tk.type != TK_COMMA)
                    {
                        status = false;
                        error("expected ',' or '}'\n");
                    }
                }
            }
        }
    }

    if(status)
    {
        *ptr = list_head;
    }

    return status;
}

bool Parser::parse_goto_stmt(Statement** ptr)
{
    bool status = true;

    strptr target = { };

    Token tk = m_stack->pop();
    if(tk.type != TK_GOTO)
    {
        status = false;
        error("expected 'goto'\n");
    }

    if(status)
    {
        tk = m_stack->pop();
        if(tk.type == TK_IDENTIFIER)
        {
            target = tk.identifier.string;
        }
        else
        {
            status = false;
            error("expected identifier\n");
        }
    }

    if(status)
    {
        if(m_stack->pop().type != TK_SEMICOLON)
        {
            status = false;
            error("expected ';'\n");
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_GOTO;
        stmt->goto_stmt.target = target;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_break_stmt(Statement** ptr)
{
    bool status = true;

    Token tk = m_stack->pop();
    if(tk.type != TK_BREAK)
    {
        status = false;
        error("expected 'break'\n");
    }

    if(status)
    {
        if(m_stack->pop().type != TK_SEMICOLON)
        {
            status = false;
            error("expected ';'\n");
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_BREAK;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_label(Statement** ptr)
{
    bool status = true;

    strptr name = {};

    Token tk = m_stack->pop();
    if(tk.type == TK_IDENTIFIER)
    {
        name = tk.identifier.string;
    }
    else
    {
        status = false;
        error("expected identifier\n");
    }

    if(m_stack->pop().type != TK_COLON)
    {
        status = false;
        error("expected ':'\n");
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_LABEL;
        stmt->label.name = name;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_continue_stmt(Statement** ptr)
{
    bool status = true;

    Token tk = m_stack->pop();
    if(tk.type != TK_CONTINUE)
    {
        status = false;
        error("expected 'continue'\n");
    }

    if(status)
    {
        if(m_stack->pop().type != TK_SEMICOLON)
        {
            status = false;
            error("expected ';'\n");
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_CONTINUE;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_switch_stmt(Statement** ptr)
{
    bool status = true;

    Expression* expr = nullptr;
    Statement*  body = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_SWITCH)
    {
        status = false;
        error("expected 'switch'\n");
    }

    if(status)
    {
        if(m_stack->pop().type != TK_OPEN_ROUND_BRACKET)
        {
            status = false;
            error("expected '('\n");
        }
    }

    if(status)
    {
        status = parse_expression(&expr);
    }

    if(status)
    {
        if(m_stack->pop().type != TK_CLOSE_ROUND_BRACKET)
        {
            status = false;
            error("expected ')'\n");
        }
    }

    if(status)
    {
        status = parse_body(&body);
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_SWITCH;
        stmt->switch_stmt.expr = expr;
        stmt->switch_stmt.body = body;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_default_stmt(Statement** ptr)
{
    bool status = true;

    Token tk = m_stack->pop();
    if(tk.type != TK_DEFAULT)
    {
        status = false;
        error("expecting 'default'\n");
    }

    if(status)
    {
        if(m_stack->pop().type != TK_COLON)
        {
            status = false;
            error("expecting ';'\n");
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_DEFAULT_CASE;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_case_stmt(Statement** ptr)
{
    bool status = true;

    Literal value = { };

    Token tk = m_stack->pop();
    if(tk.type != TK_CASE)
    {
        status = false;
        error("expecting 'case'\n");
    }

    if(status)
    {
        tk = m_stack->pop();

        if(tk.type == TK_LITERAL)
        {
            value = tk.literal;
        }
        else
        {
            status = false;
            error("expecting literal\n");
        }
    }

    if(status)
    {
        if(m_stack->pop().type != TK_COLON)
        {
            status = false;
            error("expecting ':'\n");
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_CASE;
        stmt->case_stmt.value = value;

        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_statement(Statement** ptr)
{
    bool status = true;

    Statement* stmt = nullptr;

    Token tk = m_stack->peek(0);
    switch(tk.type)
    {
        case TK_BREAK:
        {
            status = parse_break_stmt(&stmt);
            break;
        }

        case TK_CONTINUE:
        {
            status = parse_continue_stmt(&stmt);
            break;
        }

        case TK_SWITCH:
        {
            status = parse_switch_stmt(&stmt);
            break;
        }

        case TK_CASE:
        {
            status = parse_case_stmt(&stmt);
            break;
        }

        case TK_DEFAULT:
        {
            status = parse_default_stmt(&stmt);
            break;
        }

        case TK_GOTO:
        {
            status = parse_goto_stmt(&stmt);
            break;
        }

        case TK_SEMICOLON:
        {
            break;
        }

        case TK_OPEN_CURLY_BRACKET:
        {
            status = parse_compound_stmt(&stmt);
            break;
        }

        case TK_TYPEDEF:
        {
            status = parse_typedef(&stmt);
            break;
        }

        case TK_NAMESPACE:
        {
            status = parse_namespace(&stmt);
            break;
        }

        case TK_UNION:
        case TK_STRUCT:
        {
            tk = m_stack->peek(1);
            if(tk.type == TK_IDENTIFIER)
            {
                status = parse_composite_definition(&stmt);
            }
            else
            {
                goto DECLARATION;
            }
            
            break;
        }

        case TK_ENUM:
        {
            tk = m_stack->peek(1);
            if(tk.type == TK_IDENTIFIER)
            {
                status = parse_enum_definition(&stmt);
            }
            else
            {
                goto DECLARATION;
            }

            break;
        }

        DECLARATION:
        case TK_TYPE:
        case TK_CONST:
        case TK_EXTERN:
        {
            status = parse_declaration(&stmt);
            break;
        }

        case TK_IF:    { status = parse_if_stmt(&stmt);    break; }
        case TK_ELSE:  { status = parse_else_stmt(&stmt);  break; }
        case TK_FOR:   { status = parse_for_stmt(&stmt);   break; }
        case TK_WHILE: { status = parse_while_stmt(&stmt); break; }

        case TK_RETURN:
        {
            status = parse_return(&stmt);
            break;
        }

        case TK_IDENTIFIER:
        {
            if((m_stack->peek(1).type == TK_COLON) && (m_stack->peek(2).type != TK_COLON))
            {
                status = parse_label(&stmt);
            }
            else
            {
                int offset = 1;
                while(status)
                {
                    if(m_stack->peek(offset + 0).type == TK_COLON)
                    {
                        if((m_stack->peek(offset + 1).type == TK_COLON) && (m_stack->peek(offset + 2).type == TK_IDENTIFIER))
                        {
                            offset += 3;
                        }
                        else
                        {
                            status = false;
                            error("unexpected token sequence\n");
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                
                if(status)
                {
                    switch(m_stack->peek(1).type)
                    {
                        case TK_IDENTIFIER:
                        case TK_ASTERISK:
                        {
                            goto DECLARATION;
                        }

                        default:
                        {
                            goto EXPRESSION;
                        }
                    }
                }
            }
			
			break;
        }

        EXPRESSION: default:
        {
            Expression* expr = nullptr;
            if(!parse_expression(&expr))
            {
                status = false;
            }
            else
            {
                if(m_stack->pop().type != TK_SEMICOLON)
                {
                    error("Expected semicolon\n");
                    status = false;
                }
                else
                {
                    stmt = new Statement();
                    stmt->type = STMT_EXPR;
                    stmt->expr = expr;
                }
            }
        }
    }

    if(status)
    {
        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_else_stmt(Statement** ptr)
{
    bool status = true;

    Statement* stmt = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_ELSE)
    {
        status = false;
        error("expected 'else'\n");
    }

    if(status)
    {
        if(m_stack->peek(0).type == TK_IF)
        {
            status = parse_if_stmt(&stmt);

            if(status)
            {
                stmt->type = STMT_ELSE_IF;
            }
        }
        else
        {
            Statement* body = nullptr;
            status = parse_body(&body);

            if(status)
            {
                stmt = new Statement();
                stmt->type = STMT_ELSE;
                stmt->else_stmt.body = body;
            }
        }
    }

    if(status)
    {
        *ptr = stmt;
    }

    return status;
}

bool Parser::parse_if_stmt(Statement** ptr)
{
    bool status = true;
    Expression* condition = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_IF)
    {
        status = false;
        error("Unexpected token\n");
    }

    if(status)
    {
        if(m_stack->pop().type != TK_OPEN_ROUND_BRACKET)
        {
            status = false;
            printf("Expected '('\n");
        }
        else
        {
            status = parse_expression(&condition);

            if(m_stack->pop().type != TK_CLOSE_ROUND_BRACKET)
            {
                status = false;
                error("Expected ')'\n");
            }
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_IF;
        stmt->if_stmt.condition = condition;

        status = parse_body(&stmt->if_stmt.body);
        if(status)
        {
            *ptr = stmt;
        }
    }

    return status;
}

bool Parser::parse_for_stmt(Statement** ptr)
{
    bool status = true;
    
    Statement*  var  = nullptr;
    Expression* cond = nullptr;
    Expression* step = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_FOR)
    {
        status = false;
        error("Expected 'for'\n");
    }

    if(status)
    {
        if(m_stack->pop().type != TK_OPEN_ROUND_BRACKET)
        {
            status = false;
            printf("Expected '('\n");
        }
        else
        {
            // read the variable
            if(m_stack->peek(0).type != TK_SEMICOLON)
            {
                status = parse_statement(&var);

                if(status && (var->type != STMT_VARIABLE_DECL))
                {
                    status = false;
                    error("Expected variable declaration\n");
                }
            }
            else
            {
                m_stack->pop();
            }

            // read the condition
            if(status)
            {
                if(m_stack->peek(0).type != TK_SEMICOLON)
                {
                    status = parse_expression(&cond);

                    if(m_stack->pop().type != TK_SEMICOLON)
                    {
                        status = false;
                        error("Expected ';'\n");
                    }
                }
                else
                {
                    m_stack->pop();
                }
            }

            // read the step expression
            if(status)
            {
                if(m_stack->peek(0).type != TK_CLOSE_ROUND_BRACKET)
                {
                    status = parse_expression(&step);

                    if(status && (m_stack->pop().type != TK_CLOSE_ROUND_BRACKET))
                    {
                        status = false;
                        error("Expected ')'\n");
                    }
                }
                else
                {
                    m_stack->pop();
                }
            }
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_FOR;
        stmt->for_stmt.variable  = var;
        stmt->for_stmt.condition = cond;
        stmt->for_stmt.step      = step;

        status = parse_body(&stmt->for_stmt.body);
        if(status)
        {
            *ptr = stmt;
        }
    }

    return status;
}

bool Parser::parse_while_stmt(Statement** ptr)
{
    bool status = true;
    Expression* condition = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_WHILE)
    {
        status = false;
        error("Expected 'while'\n");
    }

    if(status)
    {
        if(m_stack->pop().type != TK_OPEN_ROUND_BRACKET)
        {
            status = false;
            printf("Expected '('\n");
        }
        else
        {
            status = parse_expression(&condition);

            if(m_stack->pop().type != TK_CLOSE_ROUND_BRACKET)
            {
                status = false;
                error("Expected ')'\n");
            }
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_WHILE;
        stmt->while_stmt.condition = condition;

        status = parse_body(&stmt->if_stmt.body);
        if(status)
        {
            *ptr = stmt;
        }
    }

    return status;
}

bool Parser::parse_initializer(Expression** ptr)
{
    bool status = true;
    
    Initializer::Value *head = nullptr, *tail = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_OPEN_CURLY_BRACKET)
    {
        status = false;
        error("Expected '{'\n");
    }

    if(status)
    {
        if(m_stack->peek(0).type == TK_CLOSE_CURLY_BRACKET)
        {
            m_stack->pop();
        }
        else
        {
            while(status)
            {
                Expression* expr = nullptr;
                status = parse_expression(&expr);

                if(status)
                {
                    Initializer::Value* value = new Initializer::Value();
                    value->expr = expr;

                    if(head == nullptr) { head = value; }
                    else                { tail->next = value; }
                    tail = value;
                }

                tk = m_stack->pop();
                if(tk.type == TK_CLOSE_CURLY_BRACKET)
                {
                    break;
                }
                else if(tk.type != TK_COMMA)
                {
                    status = false;
                    error("Unexpected token\n");
                }
            }
        }
    }

    if(status)
    {
        Expression* expr = new Expression();
        expr->type = EXPR_INITIALIZER;
        expr->initializer.values = head;

        *ptr = expr;
    }

    return status;
}

bool Parser::parse_cast(Expression** ptr)
{
	bool status = true;

	uint8_t cast_type = 0;
	Variable* type = nullptr;
	Expression* expr = nullptr;

	Token tk = m_stack->peek(0);
	switch(tk.type)
	{
		case TK_OPEN_ROUND_BRACKET:
		{
			m_stack->pop();

			cast_type = EXPR_OP_AUTO_CAST;
			status = parse_variable(&type);

			if (status)
			{
				if (m_stack->pop().type != TK_CLOSE_ROUND_BRACKET)
				{
					status = false;
					error("expected ')'\n");
				}
			}

			break;
		}

		case TK_STATIC_CAST:
		case TK_REINTERPRET_CAST:
		{
			tk = m_stack->pop();
			cast_type = tk.type == TK_STATIC_CAST ? EXPR_OP_STATIC_CAST : EXPR_OP_REINTERPRET_CAST;

			if (m_stack->pop().type != TK_LEFT_ARROW_HEAD) { status = false; error("expected '<'\n"); }
			else { status = parse_variable(&type); }

			if (status)
			{
				if (m_stack->pop().type != TK_RIGHT_ARROW_HEAD) { status = false; error("expected '>'\n"); }
				else if (m_stack->pop().type != TK_OPEN_ROUND_BRACKET) { status = false; error("expected '('\n"); }
				
				if (status)
				{
					status = parse_expression(&expr);

					if (status)
					{
						if (m_stack->pop().type != TK_CLOSE_ROUND_BRACKET) { status = false; error("expected ')'\n"); }
					}
				}
			}

			break;
		}

		default:
		{
			status = false;
			error("unexpected token\n");
			break;
		}
	}

	if (status)
	{
		Expression* expression = new Expression();
		expression->type = EXPR_OPERATION;
		expression->operation.op = cast_type;
		expression->operation.cast.type = type;
		expression->operation.cast.expr = expr;

		*ptr = expression;
	}

	return status;
}

bool Parser::parse_expression(Expression** ptr)
{
    bool status = true;

    ExpressionList::Entry* list_head = nullptr;
    ExpressionList::Entry* list_tail = nullptr;

    bool reading = true;
    while(status && reading)
    {
        Token tk = m_stack->peek(0);

        Expression* expr = nullptr;

        switch(tk.type)
        {
            case TK_OPEN_ROUND_BRACKET:
            {
                bool is_func_call = false;
                if(list_tail != nullptr)
				{
					if (list_tail->expr->type != EXPR_OPERATION)
					{
						is_func_call = true;
					}
					else
					{
						is_func_call = list_tail->expr->operation.op == EXPR_OP_INDEX;
					}
				}

                if(!is_func_call)
                {
					bool is_cast = false;
					switch (m_stack->peek(1).type)
					{
						case TK_TYPE:
						case TK_CONST:
						{
							is_cast = true;
							break;
						}

						default: break;
					}

					if (is_cast)
					{
						goto PARSE_CAST;
					}
					else
					{
						m_stack->pop();

						Expression* sub_expr = nullptr;
						status = parse_expression(&sub_expr);

						if (status)
						{
							if (m_stack->pop().type != TK_CLOSE_ROUND_BRACKET)
							{
								error("Invalid expression, expected close bracket\n");
								status = false;
							}
							else
							{
								expr = new Expression();
								expr->type = EXPR_SUB_EXPR;
								expr->sub_expr = sub_expr;
							}
						}
					}
                }
                else
                {
                    Argument* args = nullptr;
                    status = parse_arguments(&args);

                    if(status)
                    {
                        expr = new Expression();
                        expr->type = EXPR_OPERATION;
						expr->operation.op = EXPR_OP_FUNCTION_CALL;
                        expr->operation.call.arguments = args;
                    }
                }
                
                break;
            }

			PARSE_CAST:
			case TK_STATIC_CAST:
			case TK_REINTERPRET_CAST:
			{
				status = parse_cast(&expr);
				break;
			}

            case TK_COMMA:
            case TK_SEMICOLON:
            case TK_CLOSE_ROUND_BRACKET:
            case TK_CLOSE_SQUARE_BRACKET:
            case TK_CLOSE_CURLY_BRACKET:
            {
                reading = false;
                break;
            }

            case TK_OPEN_CURLY_BRACKET:
            {
                status = parse_initializer(&expr);
                break;
            }

            case TK_LITERAL:
            case TK_IDENTIFIER:
            {
                if(!parse_value(&expr))
                {
                    status = false;
                }
                break;
            }

            case TK_ASTERISK:
            case TK_AMPERSAND:
            {
                bool is_ptr_operator = false;
                if(list_tail == nullptr) { is_ptr_operator = true;                                    }
                else                     { is_ptr_operator = list_tail->expr->type == EXPR_OPERATION; }

                if(!is_ptr_operator)
                {
                    goto OPERATOR;
                }
                else
                {
                    m_stack->pop();
                }
                

                uint8_t op = 0;
                if(tk.type == TK_ASTERISK)       { op = EXPR_OP_DEREFERENCE; }
                else if(tk.type == TK_AMPERSAND) { op = EXPR_OP_REFERENCE;   }

                expr = new Expression();
                expr->type = EXPR_OPERATION;
                expr->operation.op = op;
                
                break;
            }

            case TK_OPEN_SQUARE_BRACKET:
            {
                m_stack->pop();
                Expression* index = nullptr;
                if(parse_expression(&index))
                {
                    if(m_stack->pop().type == TK_CLOSE_SQUARE_BRACKET)
                    {
                        expr = new Expression();
                        expr->type = EXPR_OPERATION;
                        expr->operation.op = EXPR_OP_INDEX;
                        expr->operation.rhs = index;
                    }
                    else
                    {
                        status = false;
                        error("Expected ']'\n");
                    }
                }
                else
                {
                    status = false;
                }

                break;
            }

            OPERATOR:
            case TK_PLUS: case TK_MINUS: case TK_PERCENT:
            case TK_FORWARD_SLASH: case TK_CARET:
            case TK_RIGHT_ARROW_HEAD: case TK_LEFT_ARROW_HEAD:
            case TK_EQUAL: case TK_DOT: case TK_EXPLANATION_MARK:
            {
                if(!parse_operator(&expr))
                {
                    status = false;
                }
                break;
            }

            default:
            {
                error("Unexpected token %hhu in expression\n", tk.type);
                status = false;
                break;
            }
        }

        if(status && reading)
        {
            if(expr != nullptr)
            {
                ExpressionList::Entry* entry = m_list.get_entry();
                entry->expr = expr;
                entry->next = nullptr;

                if(list_head == nullptr)
                {
                    list_head = entry;
                }
                else
                {
                    list_tail->next = entry;
                    entry->prev = list_tail;
                }

                list_tail = entry;
            }
        }
    }

    if(list_head == nullptr)
    {
        error("A fatal error occured while parsing\n");
        status = false;
    }

    if(status)
    {
        status = process_expression(list_head, ptr);
    }

    return status;
}

bool Parser::check_operator_precedence(unsigned int precedence_level, uint8_t op)
{
    // operator to precedence level map
    const static uint8_t TABLE[] =
    {
        PRECEDENCE_LEVEL_INVALID, // EXPR_OP_INVALID
        PRECEDENCE_LEVEL_6, // EXPR_OP_ADD
        PRECEDENCE_LEVEL_6, // EXPR_OP_SUB
        PRECEDENCE_LEVEL_5, // EXPR_OP_MUL
        PRECEDENCE_LEVEL_5, // EXPR_OP_DIV,
        PRECEDENCE_LEVEL_9, // EXPR_OP_LOGICAL_NOT
        PRECEDENCE_LEVEL_9, // EXPR_OP_LOGICAL_AND
        PRECEDENCE_LEVEL_9, // EXPR_OP_LOGICAL_OR
        PRECEDENCE_LEVEL_7, // EXPR_OP_BITWISE_COMPLEMENT
        PRECEDENCE_LEVEL_7, // EXPR_OP_BITWISE_XOR
        PRECEDENCE_LEVEL_7, // EXPR_OP_BITWISE_AND
        PRECEDENCE_LEVEL_7, // EXPR_OP_BITWISE_OR
        PRECEDENCE_LEVEL_7, // EXPR_OP_BITWISE_L_SHIFT
        PRECEDENCE_LEVEL_7, // EXPR_OP_BITWISE_R_SHIFT
        PRECEDENCE_LEVEL_8, // EXPR_OP_CMP_EQUAL
        PRECEDENCE_LEVEL_8, // EXPR_OP_CMP_NOT_EQUAL
        PRECEDENCE_LEVEL_8, // EXPR_OP_CMP_LESS_THAN
        PRECEDENCE_LEVEL_8, // EXPR_OP_CMP_MORE_THAN
        PRECEDENCE_LEVEL_8, // EXPR_OP_CMP_LESS_THAN_OR_EQUAL
        PRECEDENCE_LEVEL_8, // EXPR_OP_CMP_MORE_THAN_OR_EQUAL
        PRECEDENCE_LEVEL_4, // EXPR_OP_REFERENCE
        PRECEDENCE_LEVEL_4, // EXPR_OP_DEREFERENCE
        PRECEDENCE_LEVEL_9, // EXPR_OP_ASSIGN
        PRECEDENCE_LEVEL_1, // EXPR_OP_ACCESS_FIELD
        PRECEDENCE_LEVEL_1, // EXPR_OP_ARROW
        PRECEDENCE_LEVEL_2, // EXPR_OP_INDEX
		PRECEDENCE_LEVEL_3, // EXPR_OP_FUNCTION_CALL
		PRECEDENCE_LEVEL_4, // EXPR_OP_STATIC_CAST
		PRECEDENCE_LEVEL_4  // EXPR_OP_REINTERPRET_CAST
    };

    bool ret = false;

    if((op > 0) && (op < sizeof(TABLE)))
    {
        ret = (precedence_level == TABLE[op]);
    }

    return ret;
}

bool Parser::process_expression(ExpressionList::Entry* list, Expression** expr)
{
    bool status = true;
    ExpressionList::Entry* head = list;

    if(head->next != nullptr)
    {
        for(unsigned int i = PRECEDENCE_LEVEL_1; i < PRECEDENCE_LEVEL_MAX; i++)
        {
            for(ExpressionList::Entry* it = head; it != nullptr; it = it->next)
            {
                if((it->expr->type == EXPR_OPERATION) && check_operator_precedence(i, it->expr->operation.op))
                {
                    ExpressionList::Entry* prev = nullptr;
                    ExpressionList::Entry* next = nullptr;
                    uint8_t op = it->expr->operation.op;

                    switch(op)
                    {
                        case EXPR_OP_REFERENCE:
                        case EXPR_OP_DEREFERENCE:
                        {
                            next = it->next;
                            Expression* rhs = next == nullptr ? nullptr : next->expr;

                            if(rhs == nullptr)
                            {
                                status = false;
                                error("No RHS for pointer operation\n");
                            }
                            else
                            {
                                it->expr->operation.rhs = rhs;
                            }

                            break;
                        }

						case EXPR_OP_STATIC_CAST:
						case EXPR_OP_REINTERPRET_CAST:
						{
							break;
						}

						case EXPR_OP_AUTO_CAST:
						{
							next = it->next;
							Expression* rhs = next == nullptr ? nullptr : next->expr;

							if (rhs == nullptr)
							{
								status = false;
								error("rhs null in cast\n");
							}
							else
							{
								it->expr->operation.cast.expr = rhs;
							}
							break;
						}

                        case EXPR_OP_INDEX:
                        {
                            prev = it->prev;
                            Expression* lhs = prev == nullptr ? nullptr : prev->expr;

                            if(lhs == nullptr)
                            {
                                status = false;
                                error("no lhs for index operation\n");
                            }
                            else
                            {
                                it->expr->operation.lhs = lhs;
                            }
                            
                            break;
                        }

						case EXPR_OP_FUNCTION_CALL:
						{
							prev = it->prev;
							Expression* lhs = prev == nullptr ? nullptr : prev->expr;

							if (lhs == nullptr)
							{
								status = false;
								error("no lhs for function call\n");
							}
							else
							{
								it->expr->operation.call.function = lhs;
							}

							break;
						}

                        default:
                        {
                            next = it->next;
                            prev = it->prev;
                            Expression* rhs = next == nullptr ? nullptr : next->expr;
                            Expression* lhs = prev == nullptr ? nullptr : prev->expr;

                            if((rhs == nullptr) || (lhs == nullptr))
                            {
                                status = false;
                                error("rhs or lhs null in operation\n");
                            }
                            else
                            {
                                switch(op)
                                {
                                    case EXPR_OP_ACCESS_FIELD:
                                    {
                                        //bool valid = false;
                                        //valid |= rhs->type == EXPR_IDENTIFIER;
                                        //valid |= rhs->type == EXPR_OPERATION && rhs->operation.op == EXPR_OP_FUNCTION_CALL;

                                        //if(!valid)
                                        //{
                                            error("RHS of accessor must be an identifier\n");
                                            status = false;
                                        //}
                                        
                                        break;
                                    }

                                    default: { break; }
                                }

                                if(status)
                                {
                                    it->expr->operation.rhs = rhs;
                                    it->expr->operation.lhs = lhs;
                                }
                            }

                            break;
                        }
                    }

                    if(status)
                    {
                        if(next != nullptr)
                        {
                            if(next->next != nullptr) { next->next->prev = it; }
                            it->next = next->next;
                            m_list.ret_entry(next);
                        }

                        if(prev != nullptr)
                        {
                            if(prev->prev != nullptr) { prev->prev->next = it; }
                            it->prev = prev->prev;

                            if(head == prev)
                            {
                                head = it;
                            }

                            m_list.ret_entry(prev);
                        }
                    }

                }
            }
        }
    }

    // once the expression has been fully simplified, we should only have one expression left
    if(head->next != nullptr)
    {
        error("Invalid expression\n");
        status = false;
    }
    else
    {
        m_list.ret_entry(head);
    }

    // debug_print_expression(head->expr);

    if(status)
    {
        *expr = head->expr;
    }

    return status;
}

bool Parser::parse_operator(Expression** ptr)
{
    bool status = true;

    Token tk = m_stack->pop();
    
    uint8_t op = EXPR_OP_INVALID;
    switch(tk.type)
    {
        case TK_PLUS:          { op = EXPR_OP_ADD;          break; }
        case TK_ASTERISK:      { op = EXPR_OP_MUL;          break; }
        case TK_FORWARD_SLASH: { op = EXPR_OP_DIV;          break; }
        case TK_AND:           { op = EXPR_OP_LOGICAL_AND;  break; }
        case TK_OR:            { op = EXPR_OP_LOGICAL_OR;   break; }
        case TK_CARET:         { op = EXPR_OP_BITWISE_XOR;  break; }
        case TK_DOT:           { op = EXPR_OP_ACCESS_FIELD; break; }
        case TK_MINUS:
        {
            op = EXPR_OP_SUB;

            tk = m_stack->peek(0);
            if(tk.type == TK_RIGHT_ARROW_HEAD) { m_stack->pop(); op = EXPR_OP_ARROW; }

            break;
        }
        case TK_LEFT_ARROW_HEAD:
        {
            op = EXPR_OP_CMP_LESS_THAN;
            
            tk = m_stack->peek(0);
            if(tk.type == TK_LEFT_ARROW_HEAD) { m_stack->pop(); op = EXPR_OP_BITWISE_L_SHIFT;        }
            else if(tk.type == TK_EQUAL)      { m_stack->pop(); op = EXPR_OP_CMP_LESS_THAN_OR_EQUAL; }

            break;
        }
        case TK_RIGHT_ARROW_HEAD:
        {
            op = EXPR_OP_CMP_MORE_THAN;
            
            tk = m_stack->peek(0);
            if(tk.type == TK_RIGHT_ARROW_HEAD) { m_stack->pop(); op = EXPR_OP_BITWISE_R_SHIFT;        }
            else if(tk.type == TK_EQUAL)       { m_stack->pop(); op = EXPR_OP_CMP_MORE_THAN_OR_EQUAL; }

            break;
        }
        case TK_EXPLANATION_MARK:
        {
            op = EXPR_OP_LOGICAL_NOT;

            tk = m_stack->peek(0);
            if(tk.type == TK_EQUAL) { m_stack->pop(); op = EXPR_OP_CMP_NOT_EQUAL; }

            break;
        }
        case TK_EQUAL:
        {
            op = EXPR_OP_ASSIGN;

            tk = m_stack->peek(0);
            if(tk.type == TK_EQUAL) { m_stack->pop(); op = EXPR_OP_CMP_EQUAL; }

            break;
        }
        default:
        {
            error("Unknown operator");
            status = false;
            break;
        }
    }
    
    if(status)
    {
        Expression* expr = new Expression();
        expr->type = EXPR_OPERATION;
        expr->operation.op = op;

        *ptr = expr;
    }

    return status;
}

bool Parser::parse_arguments(Argument** args)
{
    bool status = true;

    Argument* head = nullptr;
    Argument* tail = nullptr;

    if(m_stack->pop().type != TK_OPEN_ROUND_BRACKET)
    {
        error("Expected '(' token\n");
        status = false;
    }
    else
    {   
        if(m_stack->peek(0).type == TK_CLOSE_ROUND_BRACKET)
        {
            m_stack->pop();
        }
        else
        {
            while(status)
            {
                Expression* expr = nullptr;

                if(!parse_expression(&expr))
                {
                    status = false;
                }
                else
                {
                    Argument* arg = new Argument();
                    arg->value = expr;
                    
                    if(head == nullptr)
                    {
                        head = arg;
                        tail = arg;
                    }
                    else
                    {
                        tail->next = arg;
                        tail = arg;
                    }

                    Token tk = m_stack->pop();
                    if(tk.type == TK_CLOSE_ROUND_BRACKET)
                    {
                        break;
                    }
                    else if(tk.type != TK_COMMA)
                    {
                        error("Unexpected token\n");
                        status = false;
                    }
                }
            }
        }
    }

    if(status)
    {
        *args = head;
    }

    return status;
}

bool Parser::parse_parameters(Parameter** params)
{
    bool status = true;
    Token tk = m_stack->pop();

    *params = nullptr;

    if(tk.type != TK_OPEN_ROUND_BRACKET)
    {
        error("Expected '('\n");
        status = false;
    }
    else
    {
        if(m_stack->peek(0).type == TK_CLOSE_ROUND_BRACKET)
        {
            m_stack->pop();
        }
        else
        {
            Parameter** p_ptr = params;

            while (status)
            {
                Variable* var = nullptr;

                status = parse_variable(&var);
                if(status)
                {
                    strptr name = {}; 

					tk = m_stack->peek(0);
                    if(tk.type == TK_IDENTIFIER)
                    {
                        tk = m_stack->pop();
                        name = tk.identifier.string;
                    }
					else if (tk.type == TK_OPEN_ROUND_BRACKET)
					{
						status = parse_function_pointer(name, &var, var);
					}
                    
                    if(m_stack->peek(0).type == TK_OPEN_SQUARE_BRACKET)
                    {
                        status = parse_array(&var);
                    }

                    if(status)
                    {
                        Parameter* p = new Parameter();
                        p->type = var;
                        p->name = name;
                        *p_ptr = p;
                        p_ptr = &p->next;

                        tk = m_stack->pop();
                        if (tk.type == TK_CLOSE_ROUND_BRACKET)
                        {
                            break;
                        }
                        else if (tk.type != TK_COMMA)
                        {
                            status = false;
                            error("Unexpected token\n");
                        }
                    }
                }
            }
        }
    }

    return status;
}

bool Parser::parse_modifiers(VariableFlags& flags)
{
    bool status = true;
    bool running = true;

    while(status && running)
    {
        Token tk = m_stack->peek(0);

        switch(tk.type)
        {
            case TK_CONST:
            {
                m_stack->pop();

                if(flags.is_constant != 1) { flags.is_constant = 1; }
                else
                {
                    error("Duplicated 'const' modifier\n");
                    status = false;
                }

                break;
            }

            case TK_EXTERN:
            {
                m_stack->pop();

                if(flags.is_external_symbol != 1) { flags.is_external_symbol = 1; }
                else
                {
                    error("Duplicated 'extern' modifier\n");
                    status = false;
                }

                break;
            }

            default:
            {
                running = false;
                break;
            }
        }
    }

    return status;
}

bool Parser::parse_variable(Variable** ptr)
{
    bool status = true;

    Token tk = {};
    Variable* head = new Variable();

    status = parse_modifiers(head->flags);

    if(status)
    {
        tk = m_stack->peek(0);
        switch(tk.type)
        {
            case TK_TYPE:
            {
                m_stack->pop();
                head->type = tk.subtype;
                break;
            }

            case TK_IDENTIFIER:
            {
                Identifier* identifier = nullptr;
                status = parse_identifier(&identifier);

                if(status)
                {
                    head->type = TYPE_UNKNOWN;
                    head->identifier = identifier;
                }
                break;
            }

            case TK_UNION:
            case TK_STRUCT:
            {
                Structure* structure = nullptr;
                status = parse_struct(&structure);

                if(status && (structure->name.len > 0))
                {
                    status = false;
                    error("inline struct cannot not have a name\n");
                }

                if(status)
                {
                    head->type = TYPE_STRUCT;
                    head->structure = structure;
                }

                break;
            }

            case TK_ENUM:
            {
                Enum* enumerator = nullptr;
                status = parse_enum(&enumerator);

                if(status && (enumerator->name.len > 0))
                {
                    status = false;
                    error("inline enum cannot have a name\n");
                }

                if(status)
                {
                    head->type = TYPE_ENUM;
                    head->enumerator = enumerator;
                }

                break;
            }

            default:
            {
                status = false;
                error("expected type or identifier\n");
            }
        }
    }

    if(status)
    {
        while(m_stack->peek(0).type == TK_ASTERISK)
        {
            m_stack->pop();

            Variable* var = new Variable();
            var->type = TYPE_PTR;
            var->flags.value = head->flags.value;
            var->pointer = head;
            
            head = var;
        }
    }

    if(status)
    {
        *ptr = head;
    }

    return status;
}

bool Parser::parse_function_decl(Variable* var, strptr name, Statement** ptr)
{
    bool status = true;
    uint8_t stmt_type = STMT_INVALID;
    Parameter* params = nullptr;

    if(var->type == TYPE_STRUCT)
    {
        if(var->structure->name.len == 0)
        {
            status = false;
            error("function cannot return unnamed struct\n");
        }
    }

    if(status)
    {
        status = parse_parameters(&params);
    }
    
    if(status)
    {
        Token tk = m_stack->peek(0);
        switch(tk.type)
        {
            case TK_OPEN_CURLY_BRACKET:
            {
                stmt_type = STMT_FUNCTION_DEF;
                break;
            }
            
            case TK_SEMICOLON:
            {
                m_stack->pop();
                stmt_type = STMT_FUNCTION_DECL;
                break;
            }

            default:
            {
                error("Expected '{' or ';' after function\n");
                status = false;
                break;
            }
        }
    }

    if(status)
    {
        Function* function = new Function();
        function->body = nullptr;
        function->params = params;
        function->ret_type = var;

        Statement* stmt = new Statement();
        stmt->type           = stmt_type;
        stmt->function.name  = name;
        stmt->function.ptr   = function;

        if((stmt->type == STMT_FUNCTION_DEF) && !parse_body(&function->body))
        {
            status = false;
        }
        else
        {
            *ptr = stmt;
        }
    }

    return status;
}

bool Parser::parse_variable_decl(Variable* var, strptr name, Statement** ptr)
{
    bool status = true;

    Token tk = m_stack->peek(0);

    switch(tk.type)
    {
        case TK_SEMICOLON:
        case TK_EQUAL:
        {
            tk = m_stack->pop();

            if(var->flags.is_external_symbol && (tk.type == TK_EQUAL))
            {
                error("Cannot initialize an external symbol\n");
                status = false;
            }

            Statement* stmt = nullptr;
            if(status)
            {
                stmt = new Statement();
                stmt->type = STMT_VARIABLE_DECL;
                stmt->variable.name = name;
                stmt->variable.type = var;
            }
            
            if(status && (tk.type == TK_EQUAL))
            {
                Expression* value = nullptr;
                if(!parse_expression(&value))
                {
                    status = false;
                }
                else
                {
                    if(m_stack->pop().type != TK_SEMICOLON)
                    {
                        error("Expected ';'\n");
                        status = false;
                    }
                    else
                    {
                        if(value != nullptr)
                        {
                            stmt->variable.value = value;
                        }
                        else
                        {
                            status = false;
                        }
                    }
                }
            }

            if(status)
            {
                *ptr = stmt;
            }

            break;
        }

        default:
        {
            error("Unexpected token\n");
            status = false;
        }
    }

    return status;
}

bool Parser::parse_array(Variable** variable)
{
    bool status = true;

    while(status && (m_stack->peek(0).type == TK_OPEN_SQUARE_BRACKET))
    {
        m_stack->pop();

        Variable* head = new Variable();
        head->flags.value = (*variable)->flags.value;
        head->array.elements = *variable;

        if(m_stack->peek(0).type == TK_CLOSE_SQUARE_BRACKET)
        {
            m_stack->pop();
            head->type = TYPE_VARIABLE_SIZED_ARRAY;
            head->array.size = nullptr;
        }
        else
        {
            head->type = TYPE_CONSTANT_SIZED_ARRAY;
            status = parse_expression(&head->array.size);

            if(status)
            {
                // pop the remaining ']'
                if(m_stack->pop().type != TK_CLOSE_SQUARE_BRACKET)
                {
                    error("Expected ']'\n");
                    status = false;
                }
            }
        }
        
        if(status)
        {
            *variable = head;
        }
    }

    return status;
}

bool Parser::parse_declaration(Statement** ptr)
{
    bool status = true;

    Token     tk   = {};
    strptr    name = {};
    Variable* var  = new Variable();

    status = parse_variable(&var);
    
    if(status)
    {
        tk = m_stack->peek(0);
		if (tk.type == TK_IDENTIFIER)
		{
			m_stack->pop();
			name = tk.identifier.string;
		}
		else if (tk.type == TK_OPEN_ROUND_BRACKET)
		{
			status = parse_function_pointer(name, &var, var);
		}
		else
        {
            error("Expected identifier\n");
            status = false;
        }
    }

    if(status)
    {
        tk = m_stack->peek(0);

        if(tk.type == TK_OPEN_ROUND_BRACKET)
        {
            status = parse_function_decl(var, name, ptr);
        }
        else
        {
            if(tk.type == TK_OPEN_SQUARE_BRACKET)
            {
                status = parse_array(&var);
            }

            if(status)
            {
                status = parse_variable_decl(var, name, ptr);
            }
        }
    }

    return status;
}

bool Parser::parse_value(Expression** ptr)
{
    bool status = true;
    
    Expression* expr = nullptr;
    Token tk = m_stack->peek(0);
    
    switch(tk.type)
    {
        case TK_LITERAL:
        {
			m_stack->pop();
			
            expr = new Expression();
            expr->type = EXPR_LITERAL;
            expr->literal = tk.literal;
            break;
        }

        case TK_IDENTIFIER:
        {
			Identifier* identifier = nullptr;
			status = parse_identifier(&identifier);	

            tk = m_stack->peek(0);
            if(tk.type == TK_OPEN_ROUND_BRACKET)
            {
                Argument* args = nullptr;

                if(!parse_arguments(&args))
                {
                    error("Failed to read arguments\n");
                    status = false;
                }
                else
                {
                    Expression* func_name = new Expression();
                    func_name->type = EXPR_IDENTIFIER;
                    func_name->identifier = identifier;

                    expr = new Expression();
                    expr->type = EXPR_OPERATION;
					expr->operation.op = EXPR_OP_FUNCTION_CALL;
                    expr->operation.call.function = func_name;
                    expr->operation.call.arguments = args;
                }
            }
            else
            {
                expr = new Expression();
                expr->type = EXPR_IDENTIFIER;
                expr->identifier = identifier;
            }

            break;
        }

        default:
        {
            error("Unexpected tokens in expression\n");
            status = false;
            break;
        }
    }

    if(status)
    {
        *ptr = expr;
    }

    return status;
}

bool Parser::parse_return(Statement** ptr)
{
    bool result = true;

    if(m_stack->pop().type != TK_RETURN)
    {
        error("Expected return keyword\n");
        result = false;
    }
    else
    {
        Expression* expr = nullptr;
        if(!parse_expression(&expr))
        {
            result = false;
        }
        else
        {
            if(m_stack->pop().type != TK_SEMICOLON)
            {
                error("Expected semicolon\n");
                result = false;
            }
            else
            {
                Statement* stmt = new Statement();
                stmt->type = STMT_RET;
                stmt->ret_stmt.expression = expr;

                *ptr = stmt;
            }
        }
    }

    return result;
}

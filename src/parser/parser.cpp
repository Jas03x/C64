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

bool Parser::parse_operator(Expression** ptr)
{
	bool status = true;

	Expression* expr = new Expression();

	Token tk = m_stack->pop();
	switch (tk.type)
	{
		case TK_PLUS:
		{
			expr->type = EXPR_OPERATION;
			expr->operation.op = EXPR_OP_ADD;
			break;
		}
		case TK_MINUS:
		{
			expr->type = EXPR_OPERATION;
			expr->operation.op = EXPR_OP_SUB;
			break;
		}
		case TK_ASTERISK:
		{
			expr->type = EXPR_OPERATION;
			expr->operation.op = EXPR_OP_MUL;
			break;
		}
		case TK_FORWARD_SLASH:
		{
			expr->type = EXPR_OPERATION;
			expr->operation.op = EXPR_OP_DIV;
			break;
		}
		case TK_EQUAL:
		{
			expr->type = EXPR_OPERATION;
			expr->operation.op = EXPR_ASSIGN;
			break;
		}
		default:
		{
			status = false;
			error("expected operator\n");
		}
	}

	if (status)
	{
		*ptr = expr;
	}

	return status;
}

uint8_t Parser::get_operator_precedence(uint8_t op)
{
	const static uint8_t TABLE[] =
	{
		 0, // EXPR_OP_INVALID
		10, // EXPR_OP_ADD
		10, // EXPR_OP_SUB
		11, // EXPR_OP_MUL
		11, // EXPR_OP_DIV
		11, // EXPR_OP_MOD
		12, // EXPR_OP_INCREMENT
		12, // EXPR_OP_DECREMENT
		12, // EXPR_OP_LOGICAL_NOT
		 3, // EXPR_OP_LOGICAL_AND
		 2, // EXPR_OP_LOGICAL_OR
		 0, // EXPR_OP_BITWISE_COMPLEMENT
		 5, // EXPR_OP_BITWISE_XOR
		 6, // EXPR_OP_BITWISE_AND
		 4, // EXPR_OP_BITWISE_OR
		 9, // EXPR_OP_BITWISE_L_SHIFT
		 9, // EXPR_OP_BITWISE_R_SHIFT
		 7, // EXPR_OP_CMP_EQUAL
		 7, // EXPR_OP_CMP_NOT_EQUAL
		 8, // EXPR_OP_CMP_LESS_THAN
		 8, // EXPR_OP_CMP_MORE_THAN
		 8, // EXPR_OP_CMP_LESS_THAN_OR_EQUAL
		 8, // EXPR_OP_CMP_MORE_THAN_OR_EQUAL
		12, // EXPR_OP_REFERENCE
		12, // EXPR_OP_DEREFERENCE
		 1, // EXPR_OP_ASSIGN
		13, // EXPR_OP_ACCESS_FIELD
		13, // EXPR_OP_ARROW
		13  // EXPR_OP_INDEX
	};

	uint8_t ret = 0;

	if ((op > 0) && (op < sizeof(TABLE)))
	{
		ret = TABLE[op];
	}

	return ret;
}

bool Parser::parse_sub_expr(Expression** ptr)
{
    bool status = true;

    Expression* expr = nullptr;

    switch(m_stack->peek(0).type)
    {
        case TK_IDENTIFIER:
        {
            Identifier* id = nullptr;
            status = parse_identifier(&id);

            if(status)
            {
				expr = new Expression();
				expr->type = EXPR_IDENTIFIER;
				expr->identifier = id;
            }

            break;
        }

		case TK_OPEN_ROUND_BRACKET:
		{
			m_stack->pop();
			status = parse_expression(&expr);

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

		case TK_LITERAL:
		{
			Token tk = m_stack->pop();

			expr = new Expression();
			expr->type = EXPR_LITERAL;
			expr->literal = tk.literal;

			break;
		}

		default:
		{
			status = false;
			error("unexpected token\n");
		}
    }

	if (status)
	{
		if (m_stack->peek(0).type == TK_OPEN_CURLY_BRACKET)
		{
			Argument* args = nullptr;
			status = parse_arguments(&args);

			if (status)
			{
				Expression* f_call = new Expression();
				f_call->type = EXPR_FUNCTION_CALL;
				f_call->call.function = expr;
				f_call->call.arguments = args;

				expr = f_call;
			}
		}
	}

	if (status)
	{
		*ptr = expr;
	}

    return status;
}

bool Parser::process_expression(Expression** lhs, ExpressionStack* stack, uint8_t minimum_precedence)
{
	bool status = true;

	while (status)
	{
		Expression* op = stack->peek();
		if (op == nullptr) { break; }

		uint8_t precedence = get_operator_precedence(op->operation.op);
		if (precedence < minimum_precedence) { break; }

		stack->pop(); // consume the operator

		Expression* rhs = stack->pop();
		if (rhs == nullptr) { status = false; }

		while (status)
		{
			Expression* _op = stack->peek();
			if (_op == nullptr) { break; }

			uint8_t _precedence = get_operator_precedence(_op->operation.op);
			if (_precedence <= precedence) { break; }
			
			status = process_expression(&rhs, stack, _precedence);
		}

		if (status)
		{
			op->operation.lhs = *lhs;
			op->operation.rhs =  rhs;
			*lhs = op;
		}
	}

	return status;
}

bool Parser::parse_expression(Expression** ptr)
{
	bool status = true;
	ExpressionStack stack = ExpressionStack(&m_list);

	bool scanning = true;
	while (status && scanning)
	{
		Expression* expr = nullptr;
		status = parse_sub_expr(&expr);

		if (status)
		{
			stack.push(expr);

			switch (m_stack->peek(0).type)
			{
				case TK_SEMICOLON:
				case TK_COMMA:
				case TK_CLOSE_ROUND_BRACKET:
				{
					scanning = false;
					break;
				}

				default:
				{
					Expression* op = nullptr;
					status = parse_operator(&op);

					if (status)
					{
						stack.push(op);
					}

					break;
				}
			}
		}
	}

	if (status)
	{
		*ptr = stack.pop();
		if (stack.peek() != nullptr)
		{
			status = process_expression(ptr, &stack, 0);
		}
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
    Enum* enumerator = nullptr;
    bool status = parse_enumerator(&enumerator);

    if(status)
    {
        Statement* statement = new Statement();
        statement->type = STMT_ENUM_DEF;
        statement->enum_def.name = enumerator->name;
        statement->enum_def.enumerator = enumerator;

        *ptr = statement;
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

bool Parser::parse_enumerator(Enum** ptr)
{
    bool status = true;

    strptr name = {};

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
        Enum* enumerator = new Enum();
        enumerator->name = name;
        enumerator->values = list_head;

        *ptr = enumerator;
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
        case TK_ENUM:
        case TK_TYPE:
        case TK_CONST:
        case TK_EXTERN:
        {
            status = parse_def_or_decl(&stmt);
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
            SymbolTable::Entry* entry = nullptr;
            status = scan_identifier(&entry);
            
            if(status)
            {
                uint8_t type = entry->type;
                switch(entry->type)
                {
                    case SymbolTable::Entry::TYPE_TYPEDEF:
                    case SymbolTable::Entry::TYPE_COMPOSITE:
                    case SymbolTable::Entry::TYPE_ENUMERATOR:
                    {
                        status = parse_declaration(ptr);
                        break;
                    }

                    default:
                    {
                        goto EXPRESSION;
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

bool Parser::scan_identifier(SymbolTable::Entry** ptr)
{
    bool status = true;
    SymbolTable::Entry* entry = m_symbols.current_scope();

    int offset = 0;
    while(status)
    {
        Token tk = m_stack->peek(offset);

        if(tk.type == TK_IDENTIFIER)
        {
            entry = entry->search(tk.identifier.string);

            if(entry == nullptr)
            {
                status = false;
                error("unknown identifier\n");
            }
            else
            {
                if((m_stack->peek(offset + 1).type == TK_COLON) && (m_stack->peek(offset + 2).type == TK_COLON))
                {
                    switch(entry->type)
                    {
                        case SymbolTable::Entry::TYPE_NAMESPACE:
                        case SymbolTable::Entry::TYPE_COMPOSITE:
                        {
                            offset += 3;
                            break;
                        }

                        default:
                        {
                            status = false;
                            error("invalid scope accessor\n");
                            break;
                        }
                    }
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            status = false;
            error("expected 'identifier'\n");
        }
    }

    if(status)
    {
        *ptr = entry;
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
		case TK_STRUCT:
		case TK_CONST:
		case TK_TYPE:
		{
			cast_type = EXPR_STATIC_CAST;
			status = parse_variable(&type);

			if (status && (m_stack->pop().type != TK_OPEN_ROUND_BRACKET))
			{
				status = false;
				error("expected '('\n");
			}

			if (status)
			{
				status = parse_expression(&expr);
			}

			if (status && (m_stack->pop().type != TK_CLOSE_ROUND_BRACKET))
			{
				status = false;
				error("expected ')'\n");
			}

			break;
		}

		case TK_STATIC_CAST:
		case TK_REINTERPRET_CAST:
		{
			tk = m_stack->pop();
			cast_type = tk.type == TK_STATIC_CAST ? EXPR_STATIC_CAST : EXPR_REINTERPRET_CAST;

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
		expression->type = cast_type;
		expression->cast.type = type;
		expression->cast.expr = expr;

		*ptr = expression;
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

    Parameter *head = nullptr, *tail = nullptr;

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
                        Parameter* param = new Parameter();
                        param->type = var;
                        param->name = name;
                        
                        if(head == nullptr) { head = param; }
                        else                { tail->next = param; }
                        tail = param;

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

    if(status)
    {
        *params = head;
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
                Composite* composite = nullptr;
                status = parse_composite(&composite);

                if(status && (composite->name.len == 0))
                {
                    status = false;
                    error("inline structures cannot have names\n");
                }

                if(status)
                {
                    head->type = TYPE_COMPOSITE;
                    head->composite = composite;
                }

                break;
            }

            case TK_ENUM:
            {
                Enum* enumerator = nullptr;
                status = parse_enumerator(&enumerator);

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

bool Parser::parse_def_or_decl(Statement** ptr)
{
    bool status = true;

    switch(m_stack->peek(0).type)
    {
        case TK_ENUM:
        case TK_UNION:
        case TK_STRUCT:
        {
            if(m_stack->peek(0).type == TK_IDENTIFIER)
            {
                Token tk = m_stack->peek(1);
                if(tk.type == TK_OPEN_CURLY_BRACKET)
                {
                    status = parse_definition(ptr);
                    break;
                }
                else if(tk.type == TK_SEMICOLON)
                {
                    status = false;
                    error("TODO: DO DEM FORWARD DECLARATIONS\n");
                    break;
                }
            }

            goto DEFAULT;
        }

        DEFAULT: default:
        {
            status = parse_declaration(ptr);
        }
    }

    return status;
}

bool Parser::parse_definition(Statement** ptr)
{
    bool status = true;

    switch(m_stack->peek(0).type)
    {
        case TK_ENUM:
        {
            status = parse_enum_definition(ptr);
            break;
        }

        default:
        {
            status = parse_composite_definition(ptr);
            break;
        }
    }

    if(m_stack->pop().type != TK_SEMICOLON)
    {
        status = false;
        error("expected ';'\n");
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
        switch(m_stack->peek(0).type)
        {
            case TK_IDENTIFIER:
            {
                tk = m_stack->pop();
			    name = tk.identifier.string;
                break;
            }

            case TK_OPEN_ROUND_BRACKET:
            {
                status = parse_function_pointer(name, &var, var);
                break;
            }

            default:
            {
                error("unexpected token\n");
                status = false;
            }
        }
    }

    if(status)
    {
		tk = m_stack->peek(0);
		if (tk.type == TK_OPEN_ROUND_BRACKET)
		{
			status = parse_function_decl(var, name, ptr);
		}
		else
		{
			if (tk.type == TK_OPEN_SQUARE_BRACKET)
			{
				status = parse_array(&var);
			}

			if (status)
			{
				status = parse_variable_decl(var, name, ptr);
			}
		}
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

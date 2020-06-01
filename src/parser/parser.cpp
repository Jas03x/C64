#include <parser.hpp>

static const char* ERROR_BAD_EXPRESSION = "malformed expression";

Parser::Parser(TokenStack& stack)
{
    m_status = true;
    m_stack = &stack;
}

AST* Parser::Parse(TokenStack& stack)
{
    bool status = true;

    Parser parser(stack);
    AST* ast = new AST();
    
    while(status)
    {
        if(stack.peek().type == TK_EOF)
        {
            break;
        }
        else
        {
            Statement* stmt = nullptr;
            if(parser.parse_global_statement(&stmt))
            {
                ast->statements.insert(stmt);
            }
            else
            {
                status = false;
            }
        }
    }

    if(!status)
    {
        delete_ast(ast);
        ast = nullptr;
    }

    return ast;
}

bool Parser::accept(uint8_t type)
{
    bool ret = false;
    if(m_stack->peek().type == type)
    {
        ret = true;
    }
    return ret;
}

void Parser::error(const char* msg)
{
    m_status = false;
    puts(msg);
}

void Parser::unexpected_token(uint8_t tk, uint8_t ex)
{
    const char* TK_TBL[] =
    {
        "INVALID", // TK_INVALID
        "CONST", // TK_CONST
        "EXTERN", // TK_EXTERN
        "STRUCT", // TK_STRUCT
        "RETURN", // TK_RETURN
        "IF", // TK_IF
        "=", // TK_EQUAL
        "<", // TK_LEFT_ARROW_HEAD
        ">", // TK_RIGHT_ARROW_HEAD
        "+", // TK_PLUS
        "-", // TK_MINUS
        ".", // TK_DOT
        "*", // TK_ASTERISK
        "/", // TK_FORWARD_SLASH
        "{", // TK_OPEN_CURLY_BRACKET
        "}", // TK_CLOSE_CURLY_BRACKET
        "(", // TK_OPEN_ROUND_BRACKET
        ")", // TK_CLOSE_ROUND_BRACKET
        "[", // TK_OPEN_SQUARE_BRACKET
        "]", // TK_CLOSE_SQUARE_BRACKET
        ";", // TK_SEMICOLON
        "LITERAL", // TK_LITERAL
        "IDENTIFIER", // TK_IDENTIFIER
        ",", // TK_COMMA
        "OR", // TK_OR
        "AND", // TK_AND
        "^", // TK_CARET
        "~", // TK_TILDE
        "!", // TK_EXPLANATION_MARK
        "&", // TK_AMPERSAND
        "|", // TK_VERTICAL_BAR
        "%", // TK_PERCENT
        "TYPE", // TK_TYPE
        "FOR", // TK_FOR
        "WHILE", // TK_WHILE
        ":", // TK_COLON
        "TYPEDEF", // TK_TYPEDEF
        "BREAK", // TK_BREAK
        "GOTO", // TK_GOTO
        "ELSE", // TK_ELSE
        "CONTINUE", // TK_CONTINUE
        "SWITCH", // TK_SWITCH
        "UNION", // TK_UNION
        "CASE", // TK_CASE
        "DEFAULT", // TK_DEFAULT
        "ENUM", // TK_ENUM
        "STATIC_CAST", // TK_STATIC_CAST
        "REINTERPRET_CAST", // TK_REINTERPRET_CAST
        "EOF" // TK_EOF
    };

    const char* _tk = ((tk > 0) && (tk < TK_COUNT)) ? TK_TBL[tk] : TK_TBL[0];

    m_status = false;
    if(ex == 0)
    {
        printf("error: unexpected token '%s'\n", _tk);
    }
    else
    {
        const char* _ex = (ex < TK_COUNT) ? TK_TBL[ex] : TK_TBL[0];
        printf("error: expected token '%s' but got '%s'\n", _ex, _tk);
    }
}

bool Parser::expect(uint8_t type)
{
    Token tk = m_stack->pop();
    if(tk.type != type)
    {
        unexpected_token(tk.type, type);
    }
    return m_status;
}

bool Parser::parse_global_statement(Statement** ptr)
{
    parse_statement(ptr);
    return m_status;
}

bool Parser::parse_declaration(Statement** ptr)
{
    Type* type = nullptr;
    Type::Flags flags = {};

    parse_type_flags(&flags);

    if (accept(TK_STRUCT) || accept(TK_UNION))
    {
        Statement* comp_def = nullptr;
        if (parse_composite_declaration(&comp_def))
        {
            if (accept(TK_SEMICOLON))
            {
                *ptr = comp_def;
                m_stack->pop();
            }
            else
            {
                if (comp_def->type == STMT_COMPOSITE_DEF)
                {
                    type = new Type();
                    type->type = TYPE_COMPOSITE;
                    type->flags.all = flags.all;
                    type->data.composite = comp_def->data.composite;

                    Statement* def = nullptr;
                    if (parse_declaration(type, &def))
                    {
                        Statement* cmp_stmt = new Statement();
                        cmp_stmt->type = STMT_COMPOUND_STMT;
                        cmp_stmt->data.compound_stmt.statements.insert(comp_def);
                        cmp_stmt->data.compound_stmt.statements.insert(def);

                        *ptr = cmp_stmt;
                    }
                }
                else
                {
                    unexpected_token(m_stack->peek().type, TK_SEMICOLON);
                }
            }
        }
    }
    else
    {
        if (parse_type(&type))
        {
            type->flags.all = flags.all;
            parse_declaration(type, ptr);
        }
    }

    return m_status;
}

bool Parser::parse_declaration(Type* base_type, Statement** ptr)
{
    Type* type = nullptr;
    strptr name = {};
    parse_type(base_type, &type, &name);

    if (m_status)
    {
        parse_identifier(&name);
    }

    if (m_status)
    {
        if (accept(TK_OPEN_ROUND_BRACKET))
        {
            parse_function_declaration(type, name, ptr);
        }
        else
        {
            List<Statement::Variable> variables = {};
            while (m_status)
            {
                Statement::Variable* var = nullptr;
                if (parse_variable_definition(type, name, &var))
                {
                    variables.insert(var);
                }

                if (accept(TK_SEMICOLON))
                {
                    m_stack->pop();
                    break;
                }
                else if(expect(TK_COMMA))
                {
                    if (parse_type(base_type, &type))
                    {
                        parse_identifier(&name);
                    }
                }
            }

            if (m_status)
            {
                Statement* stmt = new Statement();
                stmt->type = STMT_VARIABLE_DECL;
                stmt->data.variable_decl.variables = variables;

                *ptr = stmt;
            }
        }
    }

    return m_status;
}

bool Parser::parse_composite_declaration(Statement** ptr)
{
    uint8_t stmt_type = STMT_INVALID;
    uint8_t comp_type = COMP_TYPE_INVALID;

    Token tk = m_stack->pop();
    switch (tk.type)
    {
        case TK_STRUCT: { comp_type = COMP_TYPE_STRUCT; break; }
        case TK_UNION:  { comp_type = COMP_TYPE_UNION;  break; }
        default:
        {
            unexpected_token(tk.type, 0);
            break;
        }
    }

    strptr comp_name = {};
    if (m_status && accept(TK_IDENTIFIER))
    {
        parse_identifier(&comp_name);
    }

    List<Statement> comp_body = {};
    if (m_status)
    {
        if (accept(TK_OPEN_CURLY_BRACKET))
        {
            m_stack->pop();
            stmt_type = STMT_COMPOSITE_DEF;
            
            while (m_status)
            {
                if (accept(TK_CLOSE_CURLY_BRACKET))
                {
                    m_stack->pop();
                    break;
                }
                else
                {
                    Statement* stmt = nullptr;
                    if (parse_statement(&stmt))
                    {
                        comp_body.insert(stmt);
                    }
                }
            }
        }
        else if(expect(TK_SEMICOLON))
        {
            
            stmt_type = STMT_COMPOSITE_DECL;
        }
    }

    if (m_status)
    {
        Composite* comp = new Composite();
        comp->type = comp_type;
        comp->name = comp_name;
        comp->body = comp_body;

        Statement* stmt = new Statement();
        stmt->type = stmt_type;
        stmt->data.composite = comp;

        *ptr = stmt;
    }

    return m_status;
}

bool Parser::parse_type_flags(Type::Flags* ptr)
{
    Type::Flags flags = {};

    while (m_status)
    {
        Token tk = m_stack->peek();
        if (tk.type == TK_CONST)
        {
            if (flags.bits.is_constant == 0)
            {
                flags.bits.is_constant = 1;
            }
            else
            {
                error("duplicate const specifier\n");
            }
        }
        else
        {
            break;
        }

        m_stack->pop(); // consume the token
    }

    if (m_status)
    {
        ptr->all = flags.all;
    }

    return m_status;
}

bool Parser::parse_type(Type** ptr)
{
    uint8_t data_type = 0;

    if(accept(TK_TYPE))
    {
        Token tk = m_stack->pop();
        switch(tk.data.subtype)
        {
            case TK_TYPE_U8:   { data_type = TYPE_U8;   break; }
            case TK_TYPE_U32:  { data_type = TYPE_U32;  break; }
            case TK_TYPE_VOID: { data_type = TYPE_VOID; break; }
            default:
            {
                error("invalid type\n");
            }
        }
    }
    else
    {
        unexpected_token(m_stack->peek().type, 0);
    }

    if (m_status)
    {
        Type* type = new Type();
        type->type = data_type;

        *ptr = type;
    }

    return m_status;
}

bool Parser::parse_type(Type* base_type, Type** ptr, strptr* name)
{
    Type* type = base_type;

    while (m_status && accept(TK_ASTERISK))
    {
        m_stack->pop();

        Type* type_ptr = new Type();
        type_ptr->type = TYPE_PTR;
        type_ptr->data.pointer = type;

        type = type_ptr;
    }

    if (m_status && accept(TK_IDENTIFIER))
    {
        parse_identifier(name);
    }
    else if (m_status && expect(TK_OPEN_ROUND_BRACKET))
    {
        Type* sub_type = nullptr;
        if (parse_type(nullptr, &sub_type, name))
        {
            if (expect(TK_CLOSE_ROUND_BRACKET))
            {
                Type* it = nullptr;
                if (sub_type->type == TYPE_FUNCTION)
                {
                    it = sub_type->data.function.ret_type;
                }
                else if (sub_type->type == TYPE_PTR)
                {
                    it = sub_type->data.pointer;
                }
                else
                {
                    error("invalid declaration\n");
                }

                while (m_status)
                {
                    if (it->data.pointer == nullptr)
                    {
                        if (accept(TK_OPEN_ROUND_BRACKET))
                        {
                            Type* func = new Type();
                            func->type = TYPE_FUNCTION;
                            func->data.function.ret_type = type;

                            it->data.pointer = func;
                            type = func;
                        }
                        else
                        {
                            it->data.pointer = type;
                            type = sub_type;
                        }

                        break;
                    }
                    else
                    {
                        it = it->data.pointer;
                        if (it->type != TYPE_PTR)
                        {
                            error("invalid declaration\n");
                        }
                    }
                }
            }
        }
    }

    if (m_status && accept(TK_OPEN_ROUND_BRACKET))
    {
        m_stack->pop();

        if (accept(TK_CLOSE_ROUND_BRACKET))
        {
            m_stack->pop();
        }
        else
        {
            while (m_status)
            {
                Function::Parameter* param = nullptr;
                if (parse_parameter(&param))
                {
                    type->data.function.parameters.insert(param);
                }

                if (accept(TK_CLOSE_CURLY_BRACKET))
                {
                    m_stack->pop();
                    break;
                }
                else
                {
                    expect(TK_COMMA);
                }
            }
        }
    }

    if (m_status)
    {
        *ptr = type;
    }

    return m_status;
}

bool Parser::parse_identifier(strptr* id)
{
    Token tk = m_stack->pop();
    if(tk.type == TK_IDENTIFIER)
    {
        *id = tk.data.identifier;
    }
    else
    {
        unexpected_token(tk.type, 0);
    }

    return m_status;
}

bool Parser::parse_function_definition(Type* ret_type, strptr name, Statement** ptr)
{
    uint8_t type = 0;
    List<Function::Parameter> params = {};
    List<Statement> body = {};

    expect(TK_OPEN_ROUND_BRACKET);
    
    if(!accept(TK_CLOSE_ROUND_BRACKET))
    {
        while(m_status)
        {
            Function::Parameter* param = nullptr;
            if(parse_parameter(&param))
            {
                params.insert(param);
            }

            if(m_status)
            {
                if(accept(TK_CLOSE_ROUND_BRACKET))
                {
                    break;
                }
                else
                {
                    expect(TK_COMMA);
                }
            }
        }
    }

    expect(TK_CLOSE_ROUND_BRACKET);

    if(m_status)
    {
        if(accept(TK_SEMICOLON))
        {
            m_stack->pop();
            type = STMT_FUNCTION_DECL;
        }
        else if(accept(TK_OPEN_CURLY_BRACKET))
        {
            type = STMT_FUNCTION_DEF;
            parse_body(&body);
        }
        else
        {
            unexpected_token(m_stack->peek().type, 0);
        }
    }

    if(m_status)
    {
        Statement* stmt = new Statement();
        stmt->type = type;
        stmt->data.function.name = name;
        stmt->data.function.ret_type = ret_type;
        stmt->data.function.parameters = params;
        stmt->data.function.body = body;

        *ptr = stmt;
    }

    return m_status;
}

bool Parser::parse_variable_definition(Type* type, strptr name, Statement::Variable** ptr)
{
    Expression* value = nullptr;
    if(accept(TK_EQUAL))
    {
        m_stack->pop();
        parse_expression(&value);
    }

    if(m_status)
    {
        Statement::Variable* var = new Statement::Variable();
        var->name = name;
        var->type = type;
        var->value = value;

        *ptr = var;
    }

    return m_status;
}

bool Parser::parse_parameter(Function::Parameter** ptr)
{
    strptr name = {};
    Type::Flags flags = {};
    Type* type = nullptr;

    if (parse_type_flags(&flags))
    {
        if (parse_type(&type))
        {
            type->flags.all = flags.all;
            parse_type(type, &type);
        }
    }

    if(m_status)
    {
        parse_identifier(&name);
    }

    if(m_status)
    {
        Function::Parameter* param = new Function::Parameter();
        param->name = name;
        param->type = type;

        *ptr = param;
    }

    return m_status;
}

bool Parser::parse_body(List<Statement>* body)
{
    expect(TK_OPEN_CURLY_BRACKET);

    while(m_status)
    {
        if (accept(TK_CLOSE_CURLY_BRACKET))
        {
            m_stack->pop();
            break;
        }
        else
        {
            Statement* stmt = nullptr;
            if (parse_statement(&stmt))
            {
                body->insert(stmt);
            }
        }
    }

    return m_status;
}

bool Parser::parse_compound_stmt(Statement** ptr)
{
    expect(TK_OPEN_CURLY_BRACKET);

    List<Statement> body = {};
    while(m_status)
    {
        Statement* stmt = nullptr;
        if(accept(TK_CLOSE_CURLY_BRACKET))
        {
            break;
        }
        else if(parse_statement(&stmt))
        {
            body.insert(stmt);
        }
    }

    if(m_status)
    {
        expect(TK_CLOSE_CURLY_BRACKET);
    }

    if(m_status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_COMPOUND_STMT;
        stmt->data.compound_stmt.statements = body;

        *ptr = stmt;
    }

    return m_status;
}

bool Parser::parse_statement(Statement** ptr)
{
    Token tk = m_stack->peek();
    switch(tk.type)
    {
        case TK_FOR:    { parse_for_stmt(ptr); break; }
        case TK_WHILE:  { parse_while_stmt(ptr); break; }
        case TK_RETURN: { parse_return_statement(ptr); break; }
        case TK_IF:     { parse_if_stmt(ptr); break; }
        case TK_CONST:
        case TK_STRUCT:
        case TK_TYPE:
        {
            parse_definition(ptr);
            break;
        }
        case TK_OPEN_CURLY_BRACKET:
        {
            parse_compound_stmt(ptr);
            break;
        }
        case TK_IDENTIFIER:
        case TK_OPEN_ROUND_BRACKET:
        {
            parse_expression(ptr);
            break;
        }
        default:
        {
            unexpected_token(tk.type, 0);
        }
    }

    return m_status;
}

bool Parser::parse_return_statement(Statement** ptr)
{
    Expression* expr = nullptr;

    expect(TK_RETURN);
    
    if(m_status)
    {
        parse_expression(&expr);
    }

    if(m_status)
    {
        expect(TK_SEMICOLON);
    }

    if(m_status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_RETURN;
        stmt->data.ret_stmt.expression = expr;

        *ptr = stmt;
    }

    return m_status;
}

bool Parser::parse_for_stmt(Statement** ptr)
{
    if(expect(TK_FOR))
    {
        expect(TK_OPEN_ROUND_BRACKET);
    }

    Statement* init = nullptr;
    if(m_status)
    {
        if(accept(TK_SEMICOLON))
        {
            m_stack->pop();
        }
        else
        {
            parse_definition(&init);
        }
    }

    Expression* cond = nullptr;
    if(m_status)
    {
        if(!accept(TK_SEMICOLON))
        {
            parse_expression(&cond);
        }

        if(m_status)
        {
            expect(TK_SEMICOLON);
        }
    }

    Expression* step = nullptr;
    if(m_status)
    {
        if(!accept(TK_CLOSE_ROUND_BRACKET))
        {
            parse_expression(&step);
        }

        if(m_status)
        {
            expect(TK_CLOSE_ROUND_BRACKET);
        }
    }

    Statement* body = nullptr;
    if(m_status)
    {
        parse_statement(&body);
    }

    if(m_status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_FOR;
        stmt->data.for_loop.init = init;
        stmt->data.for_loop.cond = cond;
        stmt->data.for_loop.step = step;
        stmt->data.for_loop.body = body;

        *ptr = stmt;
    }

    return m_status;
}

bool Parser::parse_while_stmt(Statement** ptr)
{
	expect(TK_WHILE);

	Expression* expr = nullptr;
	if(m_status)
	{
		if(expect(TK_OPEN_ROUND_BRACKET))
		{
			if(parse_expression(&expr))
			{
				expect(TK_CLOSE_ROUND_BRACKET);
			}
		}
	}

	Statement* body = nullptr;
	if(m_status)
	{
        parse_statement(&body);
	}

	if(m_status)
	{
		Statement* stmt = new Statement();
		stmt->type = STMT_WHILE;
		stmt->data.while_loop.body = body;
		stmt->data.while_loop.cond = expr;

		*ptr = stmt;
	}

	return m_status;
}

bool Parser::parse_if_stmt(Statement** ptr)
{
    expect(TK_IF);

    Expression* cond = nullptr;
    if(m_status)
    {
        if(expect(TK_OPEN_ROUND_BRACKET))
        {
            if(parse_expression(&cond))
            {
                expect(TK_CLOSE_ROUND_BRACKET);
            }
        }
    }

    Statement* true_body = nullptr;
    if(m_status)
    {
        parse_statement(&true_body);
    }

    Statement* false_body = nullptr;
    if(m_status && accept(TK_ELSE))
    {
        m_stack->pop();
        parse_statement(&false_body);
    }

    if(m_status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_IF;
        stmt->data.cond_exec.condition = cond;
        stmt->data.cond_exec.on_true = true_body;
        stmt->data.cond_exec.on_false = false_body;

        *ptr = stmt;
    }

    return m_status;
}

bool Parser::parse_expression(Statement** ptr)
{
    Expression* expr = nullptr;
    if(parse_expression(&expr))
    {
        if (expect(TK_SEMICOLON))
        {
            Statement* stmt = new Statement();
            stmt->type = STMT_EXPR;
            stmt->data.expr = expr;

            *ptr = stmt;
        }
    }
    return m_status;
}

unsigned int Parser::get_op_precedence(uint8_t op)
{
    static const unsigned int TBL[] =
    {
         0, // EXPR_OP_INVALID
        10, // EXPR_OP_ADD
        10, // EXPR_OP_SUB
        11, // EXPR_OP_MUL
        11, // EXPR_OP_DIV
        11, // EXPR_OP_MOD
        13, // EXPR_OP_INCREMENT
        13, // EXPR_OP_DECREMENT
        12, // EXPR_OP_LOGICAL_NOT
         3, // EXPR_OP_LOGICAL_AND
         2, // EXPR_OP_LOGICAL_OR
        12, // EXPR_OP_BITWISE_COMPLEMENT
         5, // EXPR_OP_BITWISE_XOR
         6, // EXPR_OP_BITWISE_AND
         4, // EXPR_OP_BITWISE_OR
         8, // EXPR_OP_BITWISE_L_SHIFT
         8, // EXPR_OP_BITWISE_R_SHIFT
         7, // EXPR_OP_CMP_EQUAL
         7, // EXPR_OP_CMP_NOT_EQUAL
         9, // EXPR_OP_CMP_LESS_THAN
         9, // EXPR_OP_CMP_MORE_THAN
         9, // EXPR_OP_CMP_LESS_THAN_OR_EQUAL
         9, // EXPR_OP_CMP_MORE_THAN_OR_EQUAL
        12, // EXPR_OP_REFERENCE
        12, // EXPR_OP_DEREFERENCE
         1, // EXPR_OP_ASSIGN
        13, // EXPR_OP_ACCESS_FIELD
        13, // EXPR_OP_ACCESS_FIELD_PTR
        13  // EXPR_OP_INDEX
    };

    unsigned int ret = 0;
    if((op > EXPR_OP_INVALID) && (op < EXPR_OP_COUNT))
    {
        ret = TBL[op];
    }
    return ret;
}

Expression* Parser::process_expr_operand(ExpressionStack* stack)
{
    Expression* operand = stack->pop();
    if (operand == nullptr)
    {
        error(ERROR_BAD_EXPRESSION);
    }

    if (m_status)
    {
        if (operand->type == EXPR_OPERATION) // process lhs operands
        {
            switch(operand->data.operation.op)
            {
                case EXPR_OP_REFERENCE:
                case EXPR_OP_DEREFERENCE:
                case EXPR_OP_DECREMENT:
                case EXPR_OP_INCREMENT:
                {
                    operand->data.operation.rhs = process_expr_operand(stack);
                    break;
                }
                default:
                {
                    error(ERROR_BAD_EXPRESSION);
                }
            }
        }
        else
        {
            while (m_status)
            {
                Expression* look_ahead = stack->peek();
                if ((look_ahead != nullptr) && (look_ahead->type == EXPR_FUNCTION_CALL))
                {
                    Expression* func_call = stack->pop();
                    func_call->data.func_call.function = operand;
                    operand = func_call;
                }
                else
                {
                    break;
                }
            }
        }
    }

    // process rhs operands
    while(m_status)
    {
        Expression* look_ahead = stack->peek();
        if ((look_ahead != nullptr) && (look_ahead->type == EXPR_OPERATION) && (look_ahead->data.operation.op == EXPR_OP_INCREMENT))
        {
            Expression* op = stack->pop();
            op->data.operation.lhs = operand;
            operand = op;
        }
        else
        {
            break;
        }
    }

    return operand;
}

Expression* Parser::process_expression(ExpressionStack* stack, Expression* lhs, uint8_t min)
{
    Expression* expr = (lhs == nullptr) ? process_expr_operand(stack) : lhs;

    while (m_status && !stack->is_empty())
    {
        Expression* op1 = stack->peek();
        if (op1->type != EXPR_OPERATION)
        {
            error(ERROR_BAD_EXPRESSION);
        }
        else
        {
            unsigned int op1_prec = get_op_precedence(op1->data.operation.op);
            if (op1_prec < min)
            {
                break;
            }
            else
            {
                stack->pop(); // consume the first operator
                Expression* rhs = process_expr_operand(stack);

                Expression* op2 = nullptr;
                if (!stack->is_empty())
                {
                    op2 = stack->peek();
                    if (op2->type != EXPR_OPERATION)
                    {
                        error(ERROR_BAD_EXPRESSION);
                    }
                }

                unsigned int op2_prec = (op2 == nullptr) ? 0 : get_op_precedence(op2->data.operation.op);
                if ((op2 != nullptr) && (op2_prec > op1_prec))
                {
                    rhs = process_expression(stack, rhs, op2_prec);
                }

                op1->data.operation.lhs = expr;
                op1->data.operation.rhs = rhs;
                expr = op1;
            }
        }
    }

    return expr;
}

bool Parser::parse_expression(Expression** ptr)
{
    ExpressionStack stack(&m_expr_stack);
    while(m_status)
    {
        Expression* expr = nullptr;

        while(m_status)
        {
            expr = nullptr;
            if(parse_expr_lhs_op(&expr))
            {
                if(expr == nullptr) { break; }
                else { stack.insert(expr); }
            }
        }

        if(m_status)
        {
            Token tk = m_stack->peek();
            switch(tk.type)
            {
                case TK_LITERAL:            { parse_expr_literal(&expr);    break; }
                case TK_IDENTIFIER:         { parse_expr_identifier(&expr); break; }
                case TK_OPEN_ROUND_BRACKET: { parse_sub_expr(&expr);        break; }
                default:
                {
                    unexpected_token(tk.type, 0);
                }
            }

            if(m_status)
            {
                stack.insert(expr);
            }
        }

        while(m_status)
        {
            expr = nullptr;
            if(parse_expr_rhs_op(&expr))
            {
                if(expr == nullptr) { break; }
                else { stack.insert(expr); }
            }
        }

        if(m_status)
        {
            // parse function call arguments
            while(m_status && accept(TK_OPEN_ROUND_BRACKET))
            {
                Expression* args = nullptr;
                if(parse_expr_args(&args))
                {
                    stack.insert(args);
                }
            }
        }

        if (m_status)
        {
            if (accept(TK_SEMICOLON) || accept(TK_CLOSE_ROUND_BRACKET) || accept(TK_COMMA))
            {
                break;
            }
            else
            {
                Expression* op = nullptr;
                if (parse_expr_operator(&op))
                {
                    stack.insert(op);
                }
            }
        }
    }

    if(m_status)
    {
        *ptr = process_expression(&stack, nullptr, 0);
    }

    return m_status;
}

bool Parser::parse_sub_expr(Expression** ptr)
{
    Expression* sub_expr = nullptr;
    
    expect(TK_OPEN_ROUND_BRACKET);
    if(m_status && parse_expression(&sub_expr))
    {
        expect(TK_CLOSE_ROUND_BRACKET);
    }

    if(m_status)
    {
        Expression* expr = new Expression();
        expr->type = EXPR_SUB_EXPR;
        expr->data.sub_expr = sub_expr;

        *ptr = expr;
    }
    
    return m_status;
}

bool Parser::parse_expr_lhs_op(Expression** ptr)
{
    uint8_t op = EXPR_OP_INVALID;

    Token tk = m_stack->peek();
    switch(tk.type)
    {
        case TK_AMPERSAND: { op = EXPR_OP_REFERENCE;   break; }
        case TK_ASTERISK:  { op = EXPR_OP_DEREFERENCE; break; }
        case TK_PLUS:
        {
            if(m_stack->look_ahead().type == TK_PLUS)
            {
                m_stack->pop();
                op = EXPR_OP_INCREMENT;
            }
            break;
        }
        default: { break; }
    }

    if(op != EXPR_OP_INVALID)
    {
        m_stack->pop();

        Expression* expr = new Expression();
        expr->type = EXPR_OPERATION;
        expr->data.operation.op = op;

        *ptr = expr;
    }

    return m_status;
}

bool Parser::parse_expr_rhs_op(Expression** ptr)
{
    uint8_t op = EXPR_OP_INVALID;

    Token tk = m_stack->peek();
    switch(tk.type)
    {
        case TK_PLUS:
        {
            if(m_stack->look_ahead().type == TK_PLUS)
            {
                m_stack->pop();
                op = EXPR_OP_INCREMENT;
            }
            break;
        }
        default: { break; }
    }

    if(op != EXPR_OP_INVALID)
    {
        m_stack->pop();

        Expression* expr = new Expression();
        expr->type = EXPR_OPERATION;
        expr->data.operation.op = op;

        *ptr = expr;
    }

    return m_status;
}

bool Parser::parse_expr_operator(Expression** ptr)
{
    uint8_t op = 0;

    Token tk = m_stack->pop();
    switch (tk.type)
    {
        case TK_PLUS:             { op = EXPR_OP_ADD; break; }
        case TK_ASTERISK:         { op = EXPR_OP_MUL; break; }
        case TK_FORWARD_SLASH:    { op = EXPR_OP_DIV; break; }
        case TK_PERCENT:          { op = EXPR_OP_MOD; break; }
        case TK_DOT:              { op = EXPR_OP_ACCESS_FIELD; break; }
        case TK_OR:               { op = EXPR_OP_LOGICAL_OR;   break; }
        case TK_AND:              { op = EXPR_OP_LOGICAL_AND;  break; }
        case TK_EXPLANATION_MARK: { op = EXPR_OP_LOGICAL_NOT;  break; }
        case TK_VERTICAL_BAR:     { op = EXPR_OP_BITWISE_OR;   break; }
        case TK_AMPERSAND:        { op = EXPR_OP_BITWISE_AND;  break; }
        case TK_TILDE:            { op = EXPR_OP_BITWISE_COMPLEMENT; break; }
        case TK_MINUS:
        {
            if(accept(TK_RIGHT_ARROW_HEAD))
            {
                m_stack->pop();
                op = EXPR_OP_ACCESS_FIELD_PTR;
            }
            else
            {
                op = EXPR_OP_SUB;
            }
            break;
        }
        case TK_RIGHT_ARROW_HEAD:
        {
            if(accept(TK_RIGHT_ARROW_HEAD))
            {
                m_stack->pop();
                op = EXPR_OP_BITWISE_R_SHIFT;
            }
            else if(accept(TK_EQUAL))
            {
                m_stack->pop();
                op = EXPR_OP_CMP_MORE_THAN_OR_EQUAL;
            }
            else
            {
                op = EXPR_OP_CMP_MORE_THAN;
            }
            break;
        }
        case TK_LEFT_ARROW_HEAD:
        {
            if(accept(TK_LEFT_ARROW_HEAD))
            {
                m_stack->pop();
                op = EXPR_OP_BITWISE_L_SHIFT;
            }
            else if(accept(TK_EQUAL))
            {
                m_stack->pop();
                op = EXPR_OP_CMP_LESS_THAN_OR_EQUAL;
            }
            else
            {
                op = EXPR_OP_CMP_LESS_THAN;
            }
            break;
        }
        case TK_EQUAL:
        {
            if(accept(TK_EQUAL))
            {
                m_stack->pop();
                op = EXPR_OP_CMP_EQUAL;
            }
            else
            {
                op = EXPR_OP_ASSIGN;
            }
            break;
        }
        default:
        {
            unexpected_token(tk.type, 0);
        }
    }

    if(m_status)
    {
        Expression* expr = new Expression();
        expr->type = EXPR_OPERATION;
        expr->data.operation.op = op;

        *ptr = expr;
    }

    return m_status;
}

bool Parser::parse_expr_args(Expression** ptr)
{
    expect(TK_OPEN_ROUND_BRACKET);

    List<Expression> args = {};

    if(m_status)
    {
        if(accept(TK_CLOSE_ROUND_BRACKET))
        {
            m_stack->pop();
        }
        else
        {
            while(m_status)
            {
                Expression* expr = nullptr;
                if(parse_expression(&expr))
                {
                    args.insert(expr);

                    if(accept(TK_CLOSE_ROUND_BRACKET))
                    {
                        m_stack->pop();
                        break;
                    }
                    else
                    {
                        expect(TK_COMMA);
                    }
                }
            }
        }
    }

    if(m_status)
    {
        Expression* expr = new Expression();
        expr->type = EXPR_FUNCTION_CALL;
        expr->data.func_call.arguments = args;

        *ptr = expr;
    }

    return m_status;
}

bool Parser::parse_expr_literal(Expression** ptr)
{
    Token tk = m_stack->pop();
    if(tk.type == TK_LITERAL)
    {
        Expression* expr = new Expression();
        expr->type = EXPR_LITERAL;
        expr->data.literal = tk.data.literal;

        *ptr = expr;
    }
    else
    {
        unexpected_token(tk.type, 0);
    }

    return m_status;
}

bool Parser::parse_expr_identifier(Expression** ptr)
{
    Token tk = m_stack->pop();
    if(tk.type == TK_IDENTIFIER)
    {
        Expression* expr = new Expression();
        expr->type = EXPR_IDENTIFIER;
        expr->data.identifier = tk.data.identifier;

        *ptr = expr;
    }
    else
    {
        unexpected_token(tk.type, 0);
    }

    return m_status;
}

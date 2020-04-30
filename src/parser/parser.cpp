#include <parser.hpp>

#include <stdarg.h>

static const char* ERROR_UNEXPECTED_TOKEN = "unexpected token\n";
static const char* ERROR_BAD_EXPRESSION = "malformed expression\n";

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
            Statement* stmt = new Statement();
            if(parser.parse_global_statement(stmt))
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

void Parser::error(const char* format, ...)
{
    m_status = false;

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

bool Parser::expect(uint8_t type)
{
    if(m_stack->pop().type != type)
    {
        error(ERROR_UNEXPECTED_TOKEN);
    }
    return m_status;
}

bool Parser::parse_global_statement(Statement* stmt)
{
    switch(m_stack->peek().type)
    {
        case TK_TYPE: { parse_definition(stmt); break; }
        default:
        {
            error(ERROR_UNEXPECTED_TOKEN);
        }
    }

    return m_status;
}

bool Parser::parse_definition(Statement* stmt)
{
    strptr name;
    Type* type = nullptr;

    if(parse_type(&type))
    {
        parse_identifier(&name);
    }

    if(m_status)
    {
        if(accept(TK_OPEN_ROUND_BRACKET))
        {
            parse_function_definition(type, name, stmt);
        }
    }

    return m_status;
}

bool Parser::parse_type(Type** ptr)
{
    uint8_t data_type = 0;
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

    if(accept(TK_TYPE))
    {
        switch(m_stack->pop().data.subtype)
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
        error(ERROR_UNEXPECTED_TOKEN);
    }

    if(m_status)
    {
        Type* type = new Type();
        type->type = data_type;
        // type->flags = flags;

        while(accept(TK_ASTERISK))
        {
            m_stack->pop();
            Type* type_ptr = new Type();
            type_ptr->type = TYPE_PTR;
            type_ptr->data.pointer = type;

            type = type_ptr;
        }

        type->flags = flags;
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
        error(ERROR_UNEXPECTED_TOKEN);
    }

    return m_status;
}

bool Parser::parse_function_definition(Type* ret_type, strptr name, Statement* stmt)
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
            parse_function_body(&body);
        }
        else
        {
            error(ERROR_UNEXPECTED_TOKEN);
        }
    }

    if(m_status)
    {
        Function* func = new Function();
        func->name = name;
        func->ret_type = ret_type;
        func->parameters = params;
        func->body = body;

        stmt->type = type;
        stmt->data.function = func;
    }

    return m_status;
}

bool Parser::parse_parameter(Function::Parameter** ptr)
{
    strptr name = {};
    Type* type = nullptr;
    
    if(parse_type(&type))
    {
        if(accept(TK_IDENTIFIER))
        {
            parse_identifier(&name);
        }
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

bool Parser::parse_function_body(List<Statement>* body)
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

bool Parser::parse_statement(Statement** ptr)
{
    switch(m_stack->peek().type)
    {
        case TK_RETURN:     { parse_return_statement(ptr); break; }
        case TK_IDENTIFIER: { parse_expression(ptr);       break; }
        default:
        {
            error(ERROR_UNEXPECTED_TOKEN);
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
    unsigned int ret = 0;
    switch(op)
    {
        default: { break; }
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
        if (operand->type == EXPR_OPERATION)
        {
            if ((operand->type == EXPR_OP_REFERENCE) || (operand->type == EXPR_OP_DEREFERENCE))
            {
                operand->data.operation.rhs = process_expr_operand(stack);
            }
            else
            {
                error(ERROR_BAD_EXPRESSION);
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

    return operand;
}

Expression* Parser::process_expression(ExpressionStack* stack, Expression* lhs, uint8_t min)
{
    // **p(q, r, s) + a * b(c)(d) / h - (e + f) * g
    
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
            unsigned int op1_prec = get_op_precedence(op1->type);
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

                unsigned int op2_prec = (op2 == nullptr) ? 0 : get_op_precedence(op2->type);
                if ((op2 != nullptr) && (op1_prec < op2_prec))
                {
                    rhs = process_expression(stack, rhs, op2_prec);
                }

                op1->data.operation.lhs = expr;
                op1->data.operation.rhs = rhs;
                expr = rhs;
            }
        }
    }

    return lhs;
}

bool Parser::parse_expression(Expression** ptr)
{
    ExpressionStack stack(&m_expr_stack);
    while(m_status)
    {
        Expression* expr = nullptr;
        switch(m_stack->peek().type)
        {
            case TK_LITERAL:    { parse_expr_literal(&expr);    break; }
            case TK_IDENTIFIER: { parse_expr_identifier(&expr); break; }
            default:
            {
                error(ERROR_UNEXPECTED_TOKEN);
            }
        }

        if(m_status)
        {
            stack.insert(expr);
            
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
            if (accept(TK_SEMICOLON) || accept(TK_CLOSE_ROUND_BRACKET))
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
        process_expression(&stack, nullptr, 0);
    }

    return m_status;
}

bool Parser::parse_expr_operator(Expression** ptr)
{
    switch (m_stack->peek().type)
    {
        default:
        {
            error(ERROR_UNEXPECTED_TOKEN);
        }
    }
    return m_status;
}

bool Parser::parse_expr_args(Expression** ptr)
{
    expect(TK_OPEN_ROUND_BRACKET);

    List<Expression> args = {};

    while(m_status)
    {
        Expression* expr = nullptr;
        if(parse_expression(&expr))
        {
            args.insert(expr);
        }

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
        error(ERROR_UNEXPECTED_TOKEN);
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
        error(ERROR_UNEXPECTED_TOKEN);
    }

    return m_status;
}

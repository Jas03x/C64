#include <parser.hpp>

#include <stdarg.h>

static const char* ERROR_UNEXPECTED_TOKEN = "unexpected token\n";

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
    if(accept(TK_TYPE))
    {
        switch(m_stack->pop().type)
        {
            case TK_TYPE_U32:  { data_type = TYPE_U8;   break; }
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
    List<Parameter>  params;
    List<Statement> body;

    expect(TK_OPEN_ROUND_BRACKET);
    
    while(m_status)
    {
        if(accept(TK_CLOSE_ROUND_BRACKET))
        {
            break;
        }
        else
        {
            Parameter* param = nullptr;
            if(parse_parameter(&param))
            {
                params.insert(param);
            }
        }
    }

    if(m_status)
    {
        if(accept(TK_SEMICOLON))
        {
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

bool Parser::parse_parameter(Parameter** ptr)
{
    strptr name;
    Type* type = nullptr;
    
    if(parse_type(&type))
    {
        parse_identifier(&name);
    }

    if(m_status)
    {
        Parameter* param = new Parameter();
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
        Statement* stmt = nullptr;
        if(parse_statement(&stmt))
        {
            body->insert(stmt);
        }
    }

    return m_status;
}

bool Parser::parse_statement(Statement** ptr)
{
    switch(m_stack->peek().type)
    {
        case TK_RETURN:     { parse_return_statement(ptr); break; }
        case TK_IDENTIFIER: { parse_expr_stmt(ptr);        break; }
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

bool Parser::parse_expr_stmt(Statement** ptr)
{
    return m_status;
}

bool Parser::parse_expression(Expression** ptr)
{
    switch(m_stack->peek().type)
    {
        case TK_LITERAL:    { parse_expr_literal(ptr);    break; }
        case TK_IDENTIFIER: { parse_expr_identifier(ptr); break; }
        default:
        {
            error(ERROR_UNEXPECTED_TOKEN);
        }
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


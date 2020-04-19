#include <parser.hpp>

Parser::Parser(TokenStack& stack)
{
    m_stack = &stack;
}

AST* Parser::Parse(TokenStack& stack)
{
    bool status = true;

    Parser parser(stack);
    AST* ast = new AST();
    
    while(status)
    {
        if(stack->peek().type == TK_EOF)
        {
            break;
        }
        else
        {
            Statement* stmt = new Statement();
            ast->statements.insert(stmt);

            status = parser.parse_global_statement(stmt);
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

bool Parser::expect(uint8_t type)
{
    bool status = true;
    if(m_stack->pop().type != type)
    {
        status = false;
        printf("error: unexpected token\n");
    }
    return status;
}

bool Parser::parse_global_statement(Statement* stmt)
{
    bool status = true;

    switch(m_stack->peek().type)
    {
        case TK_TYPE:
        {
            status = parse_definition(stmt);
        }
    }

    return status;
}

bool Parser::parse_definition(Statement* stmt)
{
    Type* type = nullptr;
    bool status = parse_type(&type);
    
    strptr name;
    if(status)
    {
        status = parse_identifier(&name);
    }

    if(status)
    {
        if(accept(TK_OPEN_ROUND_BRACKET))
        {
            status = parse_function_definition(type, name, stmt);
        }
    }

    return status;
}

bool Parser::parse_type(Type** ptr)
{
    bool status = true;

    uint8_t data_type = 0;
    switch(m_stack->pop())
    {
        case TK_U32:  { data_type = TYPE_U8;   break; }
        case TK_VOID: { data_type = TYPE_VOID; break; }
        default:
        {
            status = false;
            printf("error: expected type\n");
        }
    }

    if(status)
    {
        Type* type = new Type();
        type->type = data_type;

        *ptr = type;
    }

    return status;
}

bool Parser::parse_identifier(strptr* id)
{
    bool status = true;

    Token tk = m_stack->pop();
    if(tk.type == TK_IDENTIFIER)
    {
        *id = tk.data.identifier;
    }
    else
    {
        status = false;
        printf("error: expected identifier\n");
    }

    return status;
}

bool Parser::parse_function_definition(Type* ret_type, strptr name, Statement* stmt)
{
    bool status = expect(TK_OPEN_ROUND_BRACKET);

    uint8_t type = 0;
    List<Paramter>  params;
    List<Statement> body;

    while(status)
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
            else
            {
                status = false;
            }
        }
    }

    if(status)
    {
        if(accept(TK_SEMICOLON))
        {
            type = STMT_FUNCTION_DECL;
        }
        else if(accept(TK_OPEN_CURLY_BRACKET))
        {
            type = STMT_FUNCTION_DEF;
            status = parse_function_body(&body);
        }
        else
        {
            status = false;
            printf("error: unexpected token\n");
        }
    }

    if(status)
    {
        Function* func = new Function();
        func->name = name;
        func-ret_type = ret_type;
        func->parameters = params;
        func->body = body;

        stmt->type = type;
        stmt->data.function = func;
    }

    return status;
}

bool Parser::parse_parameter(Parameter** ptr)
{
    bool status = true;

    Type* type = nullptr;
    strptr name;
    
    status = parse_type(&type);
    if(status)
    {
        status = parse_identifier(&name);
    }

    if(status)
    {
        Parameter* param = new Parameter();
        param->name = name;
        param->type = type;

        *ptr = param;
    }

    return status;
}

bool Parser::parse_function_body(List<Statement>* body)
{
    bool status = expect(TK_OPEN_CURLY_BRACKET);

    while(status)
    {
        Statement* stmt = nullptr;
        if(parse_statement(&stmt))
        {
            body->insert(stmt);
        }
        else
        {
            status = false;
        }
    }

    return status;
}

bool Parser::parse_statement(Statement** ptr)
{
    bool status = true;
    return status;
}


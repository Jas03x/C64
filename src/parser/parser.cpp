#include <parser.hpp>

#include <status.hpp>

Parser::Parser(Stack* tokens)
{
    m_stack = tokens;

    m_root = new Root();
    m_func_ptr = &m_root->functions;
    m_stmt_ptr = nullptr;
}

bool Parser::parse_function(Arg& arg)
{
    bool status = true;

    Token tk = m_stack->pop();
    if(tk.type != TK_OPEN_ROUND_BRACKET)
    {
        printf("Error: Expected open round bracket\n");
        status = false;
    }

    Parameter* params = nullptr;
    if(status)
    {
        Parameter** p_ptr = &params;
        while(true)
        {
            tk = m_stack->pop();
            if(tk.type == TK_CLOSE_ROUND_BRACKET)
            {
                break;
            }
            else if(tk.type == TK_TYPE)
            {
                uint8_t type = tk.subtype;
                
                tk = m_stack->pop();
                if(tk.type == TK_IDENTIFIER)
                {
                    strptr name = tk.identifier.string;

                    Parameter* p = new Parameter();
                    p->type = type;
                    p->name = name;
                    *p_ptr = p;
                    p_ptr = &p->next;
                }
                else
                {
                    status = false;
                    printf("Error: Expected identifier\n");
                }

                break;
            }
            else
            {
                printf("Error: Unexpected token\n");
                status = false;
                break;
            }
        }
    }

    Function* func = new Function();
    func->ret_type = arg.decl.type;
    func->name = arg.decl.name;
    func->params = params;

    *m_func_ptr = func;
    m_func_ptr = &func->next;
    m_stmt_ptr = &func->body; 

    return status;
}

Expression* Parser::read_expression()
{
}

bool Parser::parse_variable(Arg& arg)
{
    bool status = true;

    Expression* value = nullptr;

    Token tk = m_stack->pop();
    if(tk.type == TK_EQUAL)
    {
        value = read_expression();
    }
    else if(tk.type != TK_SEMICOLON)
    {
        status = false;
        printf("Error: Unexpected token\n");
    }

    Statement* stmt = new Statement();
    stmt->type = STMT_DECL_VAR;
    stmt->decl_var.name  = arg.decl.name;
    stmt->decl_var.type  = arg.decl.type;
    stmt->decl_var.value = value;

    *m_stmt_ptr = stmt;
    m_stmt_ptr = &stmt->next;

    return status;
}

bool Parser::parse_decl()
{
    bool status = true;

    Arg arg = {};
    
    Token tk = m_stack->pop();
    if((tk.type != TK_TYPE) || (tk.subtype == TYPE_INVALID))
    {
        printf("Error: Expected type\n");
        status = false;
    }
    else
    {
        arg.decl.type = tk.subtype;
    }
    
    if(status)
    {
        tk = m_stack->pop();
        if(tk.type != TK_IDENTIFIER)
        {
            printf("Error: Expected identifier\n");
            status = false;
        }
        else
        {
            arg.decl.name = tk.identifier.string;
        }
    }

    if(status)
    {
        tk = m_stack->peek(0);
        switch(tk.type)
        {
            case TK_OPEN_ROUND_BRACKET:
            {
                status = parse_function(arg);
                break;
            }

            case TK_EQUAL:
            case TK_SEMICOLON:
            {
                status = parse_variable(arg);
                break;
            }

            default:
            {
                printf("Error: Unexpected token\n");
                status = false;
            }
        }
    }

    return status;
}

bool Parser::process()
{
    STATUS status = STATUS_WORKING;

    while(status == STATUS_WORKING)
    {
        Token tk = m_stack->peek(0);
        switch(tk.type)
        {
            case TK_TYPE:
            {
                status = parse_decl() ? STATUS_WORKING : STATUS_ERROR;
                break;
            }

            case TK_EOF:
            {
                status = STATUS_SUCCESS;
                break;
            }

            default:
            {
                m_stack->pop();
                break;
            }
        }
    }

    return (status == STATUS_SUCCESS);
}

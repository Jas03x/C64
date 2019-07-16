#include <parser.hpp>

#include <status.hpp>

Parser::Parser(TokenStack* tokens)
{
    m_stack = tokens;

    m_root = new Root();
    m_func_ptr  = nullptr;
    m_func_tail = nullptr;
    m_stmt_tail = nullptr;
}

bool Parser::parse_function(Arg& arg)
{
    printf("parse_function\n");
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

    if(status)
    {
        Function* func = new Function();
        func->ret_type = arg.decl.type;
        func->name = arg.decl.name;
        func->params = params;
        printf("Function: %.*s\n", func->name.len, func->name.ptr); 

        status = insert_function(func);
    }

    return status;
}

bool Parser::insert_function(Function* func)
{
    bool status = true;

    if(m_func_ptr != nullptr)
    {
        printf("Error: Cannot declare function inside function\n");
        status = false;
    }
    else
    {
        if(m_func_tail == nullptr)
        {
            m_root->functions = func;
        }
        else
        {
            m_func_tail->next = func;   
        }

        m_func_tail = func;
        m_func_ptr  = func;
    }

    return status;
}

bool Parser::insert_statement(Statement* stmt)
{
    bool status = true;

    if(m_func_ptr == nullptr)
    {
        printf("Error: Cannot put statements outside function\n");
        status = false;
    }
    else
    {
        if(m_stmt_tail == nullptr)
        {
            m_func_ptr->body = stmt;
        }
        else
        {
            m_stmt_tail->next = stmt;
        }

        m_stmt_tail = stmt;
    }

    return status;
}

bool Parser::read_arguments(Argument** args)
{
    bool result = true;

    Argument* head = nullptr;
    Argument* tail = nullptr;

    if(m_stack->pop().type != TK_OPEN_ROUND_BRACKET)
    {
        printf("Error: Expected '(' token\n");
        result = false;
    }
    else
    {   
        if(m_stack->peek(0).type == TK_CLOSE_ROUND_BRACKET)
        {
            m_stack->pop();
        }
        else
        {
            while(true)
            {
                Expression* expr = read_expression();

                if(expr == nullptr)
                {
                    printf("Error: Expected expression\n");
                    result = false;
                    break;
                }
                
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
                    printf("Unexpected token\n");
                    result = false;
                    break;
                }
            }
        }
    }

    if(result)
    {
        *args = head;
    }

    return result;
}

Expression* Parser::read_value()
{
    Expression* expr = nullptr;

    Token tk = m_stack->pop();
    switch(tk.type)
    {
        case TK_LITERAL:
        {
            expr = new Expression();
            expr->type = EXPR_LITERAL;
            expr->literal = tk.literal;
            break;
        }

        case TK_IDENTIFIER:
        {
            strptr name = tk.identifier.string;

            tk = m_stack->peek(0);
            if(tk.type == TK_OPEN_ROUND_BRACKET)
            {
                Argument* args = nullptr;

                if(!read_arguments(&args))
                {
                    printf("Error reading arguments\n");
                }
                else
                {
                    expr = new Expression();
                    expr->type = EXPR_CALL;
                    expr->call.func_name = name;
                    expr->call.arguments = args;
                }
            }
            else
            {
                expr = new Expression();
                expr->type = EXPR_IDENTIFIER;
                expr->identifier.name = name;
            }

            break;
        }

        default:
        {
            printf("Error: Unexpected tokens in expression\n");
            break;
        }
    }

    return expr;
}

bool Parser::check_operator_precedence(unsigned int precedence_level, uint8_t op)
{
    bool ret = false;

    switch(precedence_level)
    {
        case PRECEDENCE_LEVEL_1:
        {
            ret = (op == EXPR_OP_MUL) || (op == EXPR_OP_DIV);
            break;
        }

        case PRECEDENCE_LEVEL_2:
        {
            ret = (op == EXPR_OP_ADD) || (op == EXPR_OP_SUB);
            break;
        }

        default: break;
    }

    return ret;
}

Expression* Parser::process_expression(ExpressionList::Entry* expression)
{
    ExpressionList::Entry* head = expression;

    for(unsigned int i = PRECEDENCE_LEVEL_1; i < PRECEDENCE_LEVEL_MAX; i++)
    {
        for(ExpressionList::Entry* it = head; it != nullptr; it = it->next)
        {
            if((it->expr->type == EXPR_OPERATION) && check_operator_precedence(i, it->expr->operation.op))
            {   
                ExpressionList::Entry* prev = it->prev;
                ExpressionList::Entry* next = it->next;
                if((prev == nullptr) || (next == nullptr)) { printf("ERROR\n"); return nullptr; }

                Expression* lhs = prev->expr;
                Expression* rhs = next->expr;
                if((lhs == nullptr) || (rhs == nullptr)) { printf("ERROR\n"); return nullptr; }

                it->expr->operation.lhs = lhs;
                it->expr->operation.rhs = rhs;

                if(prev->prev != nullptr) { prev->prev->next = it; }
                if(next->next != nullptr) { next->next->prev = it; }

                it->prev = prev->prev;
                it->next = next->next;

                if(head == prev)
                {
                    head = it;
                }

                m_list.ret_entry(prev);
                m_list.ret_entry(next);
            }
        }
    }

    // debug_print_expression(head->expr);

    return head->expr;
}

Expression* Parser::read_expression()
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
                m_stack->pop();
                expr = read_expression();
                if(expr == nullptr)
                {
                    status = false;
                }
                else if(m_stack->pop().type != TK_CLOSE_ROUND_BRACKET)
                {
                    printf("Invalid expression: Expected close bracket\n");
                    status = false;
                }
                break;
            }

            case TK_COMMA:
            case TK_SEMICOLON:
            case TK_CLOSE_ROUND_BRACKET:
            {
                reading = false;
                break;
            }

            case TK_LITERAL:
            case TK_IDENTIFIER:
            {
                expr = read_value();
                if(expr == nullptr)
                {
                    status = false;
                }
                break;
            }

            case TK_PLUS:
            case TK_MINUS:
            case TK_ASTERISK:
            case TK_FORWARD_SLASH:
            {
                m_stack->pop();

                expr = new Expression();
                expr->type = EXPR_OPERATION;
                switch(tk.type)
                {
                    case TK_PLUS:          { expr->operation.op = EXPR_OP_ADD; break; }
                    case TK_MINUS:         { expr->operation.op = EXPR_OP_SUB; break; }
                    case TK_ASTERISK:      { expr->operation.op = EXPR_OP_MUL; break; }
                    case TK_FORWARD_SLASH: { expr->operation.op = EXPR_OP_DIV; break; }
                    default:               { break; }
                }
                break;
            }

            default:
            {
                printf("Unexpected token in expression\n");
                status = false;
                break;
            }
        }

        if(status && reading)
        {
            if(expr == nullptr)
            {
                printf("Error reading expression\n");
                status = false;
            }
            else
            {
                ExpressionList::Entry* entry = m_list.get_entry();
                entry->expr = expr;
                entry->next = nullptr;

                if(list_head == nullptr)
                {
                    list_head = entry;
                    list_tail = entry;
                }
                else
                {
                    list_tail->next = entry;
                    entry->prev = list_tail;
                    list_tail = entry;
                }
            }
            
        }
    }

    Expression* expr = process_expression(list_head);
    return expr;
}

bool Parser::parse_variable(Arg& arg)
{
    bool status = true;

    Expression* value = nullptr;

    Token tk = m_stack->pop();
    if(tk.type == TK_EQUAL)
    {
        value = read_expression();

        if(value == nullptr)
        {
            printf("Error: Expected value\n");
            status = false;
        }
        else
        {
            tk = m_stack->pop();
            if(tk.type != TK_SEMICOLON)
            {
                if(tk.type == TK_COMMA)
                {
                    printf("TODO: HANDLE MULTI DECLARATIONS\n");
                }
                else
                {
                    printf("Error: Expected semicolon\n");
                    status = false;
                }
            }
        }
    }
    else if(tk.type != TK_SEMICOLON)
    {
        status = false;
        printf("Error: Unexpected token\n");
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_DECL_VAR;
        stmt->decl_var.name  = arg.decl.name;
        stmt->decl_var.type  = arg.decl.type;
        stmt->decl_var.value = value;

        status = insert_statement(stmt);
    }

    return status;
}

bool Parser::parse_decl()
{
    printf("parse_decl\n");
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

bool Parser::handle_end_of_block()
{
    bool result = true;

    if(m_func_ptr == nullptr)
    {
        printf("Error: Unexpected end of block\n");
        result = false;
    }
    else
    {
        m_func_ptr  = nullptr;
        m_stmt_tail = nullptr;
    }

    return result;
}

bool Parser::parse_return()
{
    bool result = true;

    if(m_stack->pop().type != TK_RETURN)
    {
        printf("ERROR: Expected return keyword\n");
        result = false;
    }
    else
    {
        Expression* expr = read_expression();
        if(expr == nullptr)
        {
            result = false;
        }
        else
        {
            if(m_stack->pop().type != TK_SEMICOLON)
            {
                printf("ERROR: Expected semicolon\n");
                result = false;
            }
            else
            {
                Statement* stmt = new Statement();
                stmt->type = STMT_RET;
                stmt->ret_stmt.expression = expr;

                result = insert_statement(stmt);
            }
        }
    }

    return result;
}

bool Parser::parse_statement()
{
    bool result = true;

    Token tk = m_stack->peek(0);
    switch(tk.type)
    {
        case TK_TYPE:   { result = parse_decl();   break; }
        case TK_RETURN: { result = parse_return(); break; }
    }

    return result;
}

bool Parser::process()
{
    printf("process\n");
    STATUS status = STATUS_WORKING;

    while(status == STATUS_WORKING)
    {
        Token tk = m_stack->peek(0);
        switch(tk.type)
        {
            case TK_RETURN:
            case TK_TYPE:
            {
                status = parse_statement() ? STATUS_WORKING : STATUS_ERROR;
                break;
            }

            case TK_EOF:
            {
                status = STATUS_SUCCESS;
                break;
            }

            case TK_CLOSE_CURLY_BRACKET:
            {
                status = handle_end_of_block() ? STATUS_WORKING : STATUS_ERROR;
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

Root* Parser::get_ast()
{
    return m_root;
}

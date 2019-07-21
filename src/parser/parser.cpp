#include <parser.hpp>

#include <status.hpp>

#define error(str, ...) printf("[%s]: " str, __FUNCTION__, __VA_ARGS__)

Parser::Parser(TokenStack* tokens)
{
    m_stack = tokens;

    m_root = new Root();
    m_func_ptr  = nullptr;
    m_func_tail = nullptr;
    m_stmt_tail = nullptr;

    m_stmt_stack_idx = 0;
    memset(m_stmt_stack, 0, sizeof(m_stmt_stack));
}

bool Parser::parse_function(Arg& arg)
{
    bool status = true;

    Token tk = m_stack->pop();
    if(tk.type != TK_OPEN_ROUND_BRACKET)
    {
        error("Expected open round bracket\n");
        status = false;
    }

    Parameter* params = nullptr;
    if(status)
    {
        if(m_stack->peek(0).type == TK_CLOSE_ROUND_BRACKET)
        {
            m_stack->pop();
        }
        else
        {
            Parameter** p_ptr = &params;
            while(true)
            {
                tk = m_stack->pop();
                if(tk.type == TK_TYPE)
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

                        tk = m_stack->pop();
                        if(tk.type == TK_CLOSE_ROUND_BRACKET)
                        {
                            break;
                        }
                        else if(tk.type != TK_COMMA)
                        {
                            status = false;
                            error("Unexpected token\n");
                        }
                    }
                    else
                    {
                        status = false;
                        error("Expected parameter name\n");
                    }
                }
                else
                {
                    error("Expected parameter type\n");
                    status = false;
                }
            }
        }
    }

    if(status)
    {
        tk = m_stack->pop();
        if(tk.type != TK_OPEN_CURLY_BRACKET)
        {
            error("Expected block start after function declaration\n");
        }
    }

    if(status)
    {
        Statement* body = new Statement();
        body->type = STMT_BLOCK;

        Function* func = new Function();
        func->ret_type = arg.decl.type;
        func->name = arg.decl.name;
        func->params = params;
        func->body = body;

        status = insert_function(func);
    }
    
    return status;
}

bool Parser::insert_function(Function* func)
{
    bool status = true;

    if(m_func_ptr != nullptr)
    {
        error("Cannot declare function inside function\n");
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

        insert_statement(func->body);
    }

    return status;
}

bool Parser::insert_statement(Statement* stmt)
{
    bool status = true;

    if(m_func_ptr == nullptr)
    {
        error("Cannot put statements outside function\n");
        status = false;
    }
    else
    {
        if(m_stmt_tail != nullptr)
        {
            m_stmt_tail->next = stmt;
        }

        m_stmt_tail = stmt;
        m_stmt_stack[m_stmt_stack_idx] = stmt;

        switch (stmt->type)
        {
            case STMT_IF:
            {
                m_stmt_tail = stmt->if_stmt.body;
                m_stmt_stack[++m_stmt_stack_idx] = stmt->if_stmt.body;
                break;
            }

            default:
            {
                break;
            }
        }
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
        error("Expected '(' token\n");
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
                    error("Expected expression\n");
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
                    error("Unexpected token\n");
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
                    error("Failed to read arguments\n");
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
            error("Unexpected tokens in expression\n");
            break;
        }
    }

    return expr;
}

bool Parser::check_operator_precedence(unsigned int precedence_level, uint8_t op)
{
    // operator to precedence level map
    const static uint8_t TABLE[] =
    {
        PRECEDENCE_LEVEL_INVALID, // EXPR_OP_INVALID
        PRECEDENCE_LEVEL_4, // EXPR_OP_ADD
        PRECEDENCE_LEVEL_4, // EXPR_OP_SUB
        PRECEDENCE_LEVEL_3, // EXPR_OP_MUL
        PRECEDENCE_LEVEL_3, // EXPR_OP_DIV,
        PRECEDENCE_LEVEL_7, // EXPR_OP_LOGICAL_NOT
        PRECEDENCE_LEVEL_7, // EXPR_OP_LOGICAL_AND
        PRECEDENCE_LEVEL_7, // EXPR_OP_LOGICAL_OR
        PRECEDENCE_LEVEL_5, // EXPR_OP_BITWISE_COMPLEMENT
        PRECEDENCE_LEVEL_5, // EXPR_OP_BITWISE_XOR
        PRECEDENCE_LEVEL_5, // EXPR_OP_BITWISE_AND
        PRECEDENCE_LEVEL_5, // EXPR_OP_BITWISE_OR
        PRECEDENCE_LEVEL_5, // EXPR_OP_BITWISE_L_SHIFT
        PRECEDENCE_LEVEL_5, // EXPR_OP_BITWISE_R_SHIFT
        PRECEDENCE_LEVEL_6, // EXPR_OP_CMP_EQUAL
        PRECEDENCE_LEVEL_6, // EXPR_OP_CMP_NOT_EQUAL
        PRECEDENCE_LEVEL_6, // EXPR_OP_CMP_LESS_THAN
        PRECEDENCE_LEVEL_6, // EXPR_OP_CMP_MORE_THAN
        PRECEDENCE_LEVEL_6, // EXPR_OP_CMP_LESS_THAN_OR_EQUAL
        PRECEDENCE_LEVEL_6, // EXPR_OP_CMP_MORE_THAN_OR_EQUAL
        PRECEDENCE_LEVEL_2, // EXPR_OP_REFERENCE
        PRECEDENCE_LEVEL_2, // EXPR_OP_DEREFERENCE
        PRECEDENCE_LEVEL_7, // EXPR_OP_ASSIGN
        PRECEDENCE_LEVEL_1, // EXPR_OP_ARROW
    };

    bool ret = false;

    if((op > 0) && (op < sizeof(TABLE)))
    {
        ret = (precedence_level == TABLE[op]);
    }

    return ret;
}

Expression* Parser::process_expression(ExpressionList::Entry* expression)
{
    ExpressionList::Entry* head = expression;

    if(head->next == nullptr)
    {
        return head->expr;
    }

    for(unsigned int i = PRECEDENCE_LEVEL_1; i < PRECEDENCE_LEVEL_MAX; i++)
    {
        for(ExpressionList::Entry* it = head; it != nullptr; it = it->next)
        {
            if((it->expr->type == EXPR_OPERATION) && check_operator_precedence(i, it->expr->operation.op))
            {
                bool single_operand = (it->expr->operation.op == EXPR_OP_REFERENCE) || (it->expr->operation.op == EXPR_OP_DEREFERENCE);

                ExpressionList::Entry* prev = it->prev;
                ExpressionList::Entry* next = it->next;
                Expression* lhs = prev == nullptr ? nullptr : prev->expr;
                Expression* rhs = next == nullptr ? nullptr : next->expr;

                // TODO: handle these errors properly ...
                if((next == nullptr) || (rhs == nullptr)) { error("Missing rhs operand\n"); return nullptr; }
                if(!single_operand && ((prev == nullptr) || (lhs == nullptr))) { error("Missing lhs operand\n"); return nullptr; }

                it->expr->operation.rhs = rhs;
                if(!single_operand)
                {
                    it->expr->operation.lhs = lhs;
                }

                if(next->next != nullptr) { next->next->prev = it; }
                it->next = next->next;
                m_list.ret_entry(next);

                if(!single_operand)
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

    // debug_print_expression(head->expr);

    return head->expr;
}

Expression* Parser::read_operator()
{
    bool status = true;
    Expression* expr = nullptr;

    Token tk = m_stack->pop();
    
    uint8_t op = EXPR_OP_INVALID;
    switch(tk.type)
    {
        case TK_PLUS:          { op = EXPR_OP_ADD;         break; }
        case TK_ASTERISK:      { op = EXPR_OP_MUL;         break; }
        case TK_FORWARD_SLASH: { op = EXPR_OP_DIV;         break; }
        case TK_AND:           { op = EXPR_OP_LOGICAL_AND; break; }
        case TK_OR:            { op = EXPR_OP_LOGICAL_OR;  break; }
        case TK_CARET:         { op = EXPR_OP_BITWISE_XOR; break; }
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
        expr = new Expression();
        expr->type = EXPR_OPERATION;
        expr->operation.op = op;
    }

    return expr;
}

Expression* Parser::read_expression()
{
    bool status = true;

    Expression* expr = nullptr;
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
                    error("Invalid expression, expected close bracket\n");
                    status = false;
                }
                break;
            }

            case TK_COMMA:
            case TK_SEMICOLON:
            case TK_CLOSE_ROUND_BRACKET:
            case TK_OPEN_CURLY_BRACKET:
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

            case TK_ASTERISK:
            case TK_AMPERSAND:
            {
                bool is_ptr_operator = false;
                if(list_tail == nullptr)
                {
                    is_ptr_operator = true;
                }
                else
                {
                    is_ptr_operator = list_tail->expr->type == EXPR_OPERATION;
                }

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

            OPERATOR:
            case TK_PLUS:
            case TK_MINUS:
            case TK_FORWARD_SLASH:
            case TK_RIGHT_ARROW_HEAD:
            case TK_LEFT_ARROW_HEAD:
            case TK_CARET:
            case TK_PERCENT:
            {
                expr = read_operator();
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
            if(expr == nullptr)
            {
                error("Error reading expression\n");
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

    if(status)
    {
        expr = process_expression(list_head);
    }

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
            error("Expected value\n");
            status = false;
        }
        else
        {
            tk = m_stack->pop();
            if(tk.type != TK_SEMICOLON)
            {
                if(tk.type == TK_COMMA)
                {
                    error("TODO: HANDLE MULTI DECLARATIONS\n");
                }
                else
                {
                    error("Expected semicolon\n");
                    status = false;
                }
            }
        }
    }
    else if(tk.type != TK_SEMICOLON)
    {
        status = false;
        error("Unexpected token\n");
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
    bool status = true;

    Arg arg = {};
    
    Token tk = m_stack->pop();
    if((tk.type != TK_TYPE) || (tk.subtype == TYPE_INVALID))
    {
        error("Expected type\n");
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
            error("Expected identifier\n");
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
                error("Unexpected token\n");
                status = false;
            }
        }
    }

    return status;
}

bool Parser::handle_end_of_block()
{
    Token tk = m_stack->pop();
    if(tk.type != TK_CLOSE_CURLY_BRACKET)
    {
        error("Expected '}'\n");
        return false;
    }

    bool result = true;

    if(m_func_ptr == nullptr)
    {
        error("Unexpected end of block\n");
        result = false;
    }
    else
    {
        if (m_stmt_stack_idx >= 0)
        {
            if(m_stmt_stack_idx > 0)
            {
                m_stmt_stack[m_stmt_stack_idx] = nullptr;
                m_stmt_tail = m_stmt_stack[m_stmt_stack_idx - 1];
            }
            else
            {
                m_func_ptr = nullptr;
                m_stmt_tail = nullptr;
            }
        }
        else
        {
            error("Unexpected end of block\n");
            result = false;
        }
    }

    return result;
}

bool Parser::parse_return()
{
    bool result = true;

    if(m_stack->pop().type != TK_RETURN)
    {
        error("Expected return keyword\n");
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
                error("Expected semicolon\n");
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

bool Parser::parse_if_stmt()
{
    bool status = true;
    Expression* condition = nullptr;

    Token tk = m_stack->pop();
    if(tk.type != TK_IF)
    {
        error("Unexpected token\n");
        status = false;
    }

    if(status)
    {
        if(m_stack->peek(0).type != TK_OPEN_ROUND_BRACKET)
        {
            printf("Expected '('\n");
            status = false;
        }
        else
        {
            condition = read_expression();
            status = (condition != nullptr);
        }
    }

    if(status)
    {
        if(m_stack->pop().type != TK_OPEN_CURLY_BRACKET)
        {
            error("Expected '{'\n");
            status = false;
        }
    }

    if(status)
    {
        Statement* body = new Statement();
        body->type = STMT_BLOCK;

        Statement* stmt = new Statement();
        stmt->type = STMT_IF;
        stmt->if_stmt.condition = condition;
        stmt->if_stmt.body = body;

        status = insert_statement(stmt);
    }

    return status;
}

bool Parser::parse_expr_stmt()
{
    bool status = true;

    Expression* expr = read_expression();
    if(expr == nullptr)
    {
        status = false;
    }

    if(status)
    {
        if(m_stack->pop().type != TK_SEMICOLON)
        {
            error("Expected semicolon\n");
            status = false;
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_EXPR;
        stmt->expr = expr;

        status = insert_statement(stmt);
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
            case TK_RETURN:
            case TK_TYPE:
            {
                status = parse_statement() ? STATUS_WORKING : STATUS_ERROR;
                break;
            }

            case TK_IF:
            {
                status = parse_if_stmt() ? STATUS_WORKING : STATUS_ERROR;
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
                break;
            }

            default:
            {
                status = parse_expr_stmt() ? STATUS_WORKING : STATUS_ERROR;
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

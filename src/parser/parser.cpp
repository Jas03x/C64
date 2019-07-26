#include <parser.hpp>

#include <status.hpp>

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

    AST* ast = parser.parse();
    return ast;
}

AST* Parser::parse()
{
    STATUS status = STATUS_WORKING;
    Statement *head = nullptr, *tail = nullptr;

    while(status == STATUS_WORKING)
    {
        Token tk = m_stack->peek(0);
        
        if(tk.type == TK_EOF)
        {
            m_stack->pop();

            status = STATUS_SUCCESS;
        }
        else
        {
            Statement* stmt = nullptr;
            
            if(parse_statement(&stmt))
            {
                if((stmt->type == STMT_VARIABLE)      ||
                   (stmt->type == STMT_FUNCTION_DEF)  ||
                   (stmt->type == STMT_FUNCTION_DECL) ||
                   (stmt->type == STMT_STRUCT))
                {
                    if(tail == nullptr) { head = stmt;       }
                    else                { tail->next = stmt; }
                    tail = stmt;
                }
                else
                {
                    error("invalid global statement\n");
                    status = STATUS_ERROR;
                }
            }
            else
            {
                status = STATUS_ERROR;
            }
        }
    }

    AST* ast = nullptr;
    if(status == STATUS_SUCCESS)
    {
        ast = new AST();
        ast->statements = head;
    }

    return ast;
}

bool Parser::parse_body(Statement** ptr)
{
    STATUS status = STATUS_WORKING;
    Statement *head = nullptr, *tail = nullptr;

    while(status == STATUS_WORKING)
    {
        Token tk = m_stack->peek(0);
        
        if(tk.type == TK_CLOSE_CURLY_BRACKET)
        {
            m_stack->pop();

            status = STATUS_SUCCESS;
        }
        else
        {
            Statement* stmt = nullptr;
            
            if(parse_statement(&stmt))
            {
                if(tail == nullptr) { head = stmt; }
                else                { tail->next = stmt; }
                tail = stmt;
            }
            else
            {
                status = STATUS_ERROR;
            }
        }
    }

    if(status)
    {
        *ptr = head;
    }

    return (status == STATUS_SUCCESS);
}

bool Parser::parse_struct_declaration(Statement** ptr)
{
    bool status = true;

    strptr name = {};

    Token tk = m_stack->pop();
    if(tk.type != TK_STRUCT)
    {
        error("Expected struct\n");
        status = false;
    }
    
    if(status)
    {
        tk = m_stack->pop();
        if(tk.type == TK_IDENTIFIER)
        {
            name = tk.identifier.string;
        }
        else
        {
            error("Expected identifier\n");
            status = false;
        }
    }

    Statement *head = nullptr, *tail = nullptr;

    if(status)
    {
        tk = m_stack->pop();

        if(tk.type == TK_OPEN_CURLY_BRACKET)
        {
            while(status)
            {
                if(m_stack->peek(0).type == TK_CLOSE_CURLY_BRACKET)
                {
                    m_stack->pop();
                    break;
                }

                Statement* stmt = nullptr;
                if(parse_statement(&stmt))
                {
                    if((stmt->type == STMT_VARIABLE)     ||
                       (stmt->type == STMT_FUNCTION_DEF) ||
                       (stmt->type == STMT_FUNCTION_DECL))
                    {
                        if(tail == nullptr) { head = stmt;       }
                        else                { tail->next = stmt; }
                        tail = stmt;
                    }
                    else
                    {
                        error("invalid struct member\n");
                        status = STATUS_ERROR;
                    }
                }
                else
                {
                    status = false;
                }
            }
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
        Statement* stmt = new Statement();
        stmt->type = STMT_STRUCT;
        stmt->struct_def.name = name;
        stmt->struct_def.members = head;

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
        case TK_STRUCT:
        {
            status = parse_struct_declaration(&stmt);
            break;
        }

        case TK_TYPE:
        case TK_CONST:
        case TK_EXTERN:
        {
            status = parse_declaration(&stmt);
            break;
        }

        case TK_IF:
        {
            status = parse_if_stmt(&stmt);
            break;
        }

        case TK_RETURN:
        {
            status = parse_return(&stmt);
            break;
        }

        default:
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

bool Parser::parse_if_stmt(Statement** ptr)
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
            status = parse_expression(&condition);
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
        Statement* stmt = new Statement();
        stmt->type = STMT_IF;
        stmt->if_stmt.condition = condition;

        if(parse_body(&stmt->if_stmt.body))
        {
            *ptr = stmt;
        }
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
                m_stack->pop();
                if(!parse_expression(&expr))
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
            case TK_EQUAL:
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
            if(expr == nullptr)
            {
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
                    bool single_operand = (it->expr->operation.op == EXPR_OP_REFERENCE) || (it->expr->operation.op == EXPR_OP_DEREFERENCE);

                    ExpressionList::Entry* prev = it->prev;
                    ExpressionList::Entry* next = it->next;
                    Expression* lhs = prev == nullptr ? nullptr : prev->expr;
                    Expression* rhs = next == nullptr ? nullptr : next->expr;

                    if((next == nullptr) || (rhs == nullptr))                      { error("Missing rhs operand\n"); status = false; }
                    if(!single_operand && ((prev == nullptr) || (lhs == nullptr))) { error("Missing lhs operand\n"); status = false; }

                    if(status)
                    {
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
        }
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
                DataType          type           = {};
                FunctionModifiers func_modifiers = {};

                status = parse_modifiers(type.flags, func_modifiers);
                
                if(!status) { break; }
                else if(func_modifiers.value != 0)
                {
                    status = false;
                    error("Function modifiers in parameter declaration\n");
                    break;
                }

                if(status)
                {
                    status = parse_type(type);
                }

                if(status)
                {
                    tk = m_stack->pop();
                    if (tk.type == TK_IDENTIFIER)
                    {
                        strptr name = tk.identifier.string;

                        Parameter* p = new Parameter();
                        p->type = type;
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
                    else
                    {
                        status = false;
                        error("Expected parameter name\n");
                    }
                }
            }
        }
    }

    return status;
}

bool Parser::parse_modifiers(VariableModifiers& var_mod, FunctionModifiers& func_mod)
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

                if(var_mod.is_constant != 1) { var_mod.is_constant = 1; }
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

                if(var_mod.is_external_symbol != 1) { var_mod.is_external_symbol = 1; }
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

bool Parser::parse_type(DataType& type)
{
    bool status = true;

    Token tk = m_stack->pop();
    if((tk.type != TK_TYPE) || (tk.subtype == TYPE_INVALID))
    {
        error("Expected type\n");
        status = false;
    }
    else
    {
        type.type = tk.subtype;
    }

    tk = m_stack->peek(0);
    if(tk.type == TK_ASTERISK)
    {
        m_stack->pop();
        type.flags.is_pointer = 1;
    }
    
    FixedSizeArray *head = nullptr, *tail = nullptr;

    while(true)
    {
        tk = m_stack->peek(0);
        if(tk.type != TK_OPEN_SQUARE_BRACKET)
        {
            break;
        }

        m_stack->pop();
        type.flags.is_fixed_size_array = 1;

        FixedSizeArray* array = new FixedSizeArray();

        tk = m_stack->pop();
        // TODO: support constant expressions
        if((tk.type == TK_LITERAL) && (tk.literal.type == LITERAL_INTEGER))
        {
            array->size = tk.literal.integer.value;
        }
        else
        {
            error("Expected a constant array size\n");
            status = false;
        }

        if(status)
        {
            m_stack->pop(); // pop the remaining ']'

            if(head == nullptr) { head = array;       }
            else                { tail->next = array; }

            tail = array;
        }
    }

    if(status && (head != nullptr))
    {
        type.fixed_size_array = head;
    }

    return status;
}

bool Parser::parse_declaration(Statement** ptr)
{
    bool status = true;

    Token tk = {};

    strptr   name = {};
	DataType type = {};

    FunctionModifiers func_mod = {};

    status = parse_modifiers(type.flags, func_mod);

    if(status)
    {
        status = parse_type(type);
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
            name = tk.identifier.string;
        }
    }

    if(status)
    {
        tk = m_stack->peek(0);

        switch(tk.type)
        {
            case TK_OPEN_ROUND_BRACKET:
            {
                Parameter* params = nullptr;

                if(!parse_parameters(&params))
                {
                    status = false;
                }
                else
                {
                    uint8_t stmt_type = 0;
                    
                    tk = m_stack->pop();
                    switch(tk.type)
                    {
                        case TK_OPEN_CURLY_BRACKET: { stmt_type = STMT_FUNCTION_DEF;  break; }
                        case TK_SEMICOLON:          { stmt_type = STMT_FUNCTION_DECL; break; }
                        default:
                        {
                            error("Expected '{' or ';' after function\n");
                            status = false;
                            break;
                        }
                    }

                    if(status)
                    {
                        Statement* stmt = new Statement();

                        stmt->type              = stmt_type;
                        stmt->function.ret_type = type;
                        stmt->function.name     = name;
                        stmt->function.params   = params;
                        stmt->function.body     = nullptr;

                        if((stmt->type == STMT_FUNCTION_DEF) && !parse_body(&stmt->function.body))
                        {
                            status = false;
                        }
                        else
                        {
                            *ptr = stmt;
                        }
                    }
                }

                break;
            }

			case TK_SEMICOLON:
            case TK_EQUAL:
            {
                m_stack->pop();

                if(func_mod.value != 0)
                {
                    error("Function modifiers on variable\n");
                    status = false;
                }
                else if(type.flags.is_external_symbol && (tk.type == TK_EQUAL))
                {
                    error("Cannot initialize an external symbol\n");
                    status = false;
                }

                Statement* stmt = nullptr;
                if(status)
                {
                    stmt = new Statement();
                    stmt->type = STMT_VARIABLE;
                    stmt->variable.name = name;
                    stmt->variable.type = type;
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
                            error("Expected semicolon\n");
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
    }

    return status;
}

bool Parser::parse_value(Expression** ptr)
{
    bool status = true;
    
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

                if(!parse_arguments(&args))
                {
                    error("Failed to read arguments\n");
                    status = false;
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

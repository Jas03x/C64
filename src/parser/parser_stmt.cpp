#include <parser.hpp>

#include <debug.hpp>
#include <symbol_table.hpp>

bool Parser::parse_body(list& stmt_list)
{
    bool status = true;

    if(m_stack->pop().type != TK_OPEN_CURLY_BRACKET)
    {
        status = false;
        error("expected '{'\n");
    }

    Token tk = {};
    while(status)
    {
        tk = m_stack->peek(0);
        if(tk.type == TK_CLOSE_CURLY_BRACKET)
        {
            m_stack->pop();
            break;
        }
        else
        {
            status = parse_statement(stmt_list);
        }
    }

    return status;
}

bool Parser::parse_statement(list& stmt_list)
{
    bool status = true;
    
    Token tk = m_stack->peek(0);
    switch(tk.type)
    {
        case TK_SEMICOLON: { break; }
        case TK_NAMESPACE: { status = parse_namespace(stmt_list); break; }
        case TK_GOTO:      { break; }
        case TK_RETURN:    { status = parse_return_statement(stmt_list); break; }
        case TK_BREAK:     { break; }
        case TK_SWITCH:    { break; }
        case TK_IF:        { status = parse_if_statement(stmt_list);   break; }
        case TK_FOR:       { break; }
        case TK_WHILE:     { break; }
        case TK_TYPEDEF:   { break; }
        case TK_OPEN_CURLY_BRACKET: { break; }

        case TK_UNION: case TK_STRUCT: case TK_ENUM:
        case TK_TYPE:  case TK_CONST:  case TK_EXTERN:
        {
            status = parse_definition(stmt_list);
            break;
        }

        case TK_IDENTIFIER:
        {
            switch(scan_identifier())
            {
                case SymbolTable::Entry::TYPE_TYPEDEF:
                case SymbolTable::Entry::TYPE_COMPOSITE:
                case SymbolTable::Entry::TYPE_ENUMERATOR:
                {
                    status = parse_definition(stmt_list);
                    break;
                }

                default:
                {
                    status = parse_expression(stmt_list);
                    break;
                }
            }
            break;
        }

        default:
        {
            status = false;
            error("unexpected token %s\n", token_to_str(tk));
            break;
        }
    }

    return status;
}

bool Parser::parse_expression(list& stmt_list)
{
    bool status = true;

    Expression* expr = nullptr;
    status = parse_expression(&expr);

    if(status && (m_stack->pop().type != TK_SEMICOLON)) {
        status = false;
        error("expected ';'\n");
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_EXPR;
        stmt->expr = expr;

        stmt_list.insert(stmt);
    }

    return status;
}

bool Parser::parse_definition(list& stmt_list)
{
    bool status = true;
    
    Token tk = m_stack->peek(0);
    switch(tk.type)
    {
        case TK_TYPE: case TK_CONST: case TK_IDENTIFIER:
        {
            status = parse_declaration(stmt_list);
            break;
        }

        case TK_STRUCT: case TK_UNION: case TK_ENUM:
        {
            if(m_stack->peek(1).type == TK_IDENTIFIER) {
                status = (tk.type == TK_ENUM) ? parse_enumerator_definition(stmt_list) : parse_composite_definition(stmt_list);
            } else {
                status = parse_declaration(stmt_list);
            }
            break;
        }

        default:
        {
            status = false;
            error("unexpected token %s\n", token_to_str(tk));
            break;
        }
    }

    return status;
}

bool Parser::parse_composite_definition(list& stmt_list)
{
    bool status = true;
    Statement stmt = {};

    if((m_stack->peek(1).type == TK_IDENTIFIER) && (m_stack->peek(2).type == TK_SEMICOLON))
    {
        stmt.type = STMT_COMP_DECL;
        stmt.composite = new Composite();

        Token c_type = m_stack->pop();
        Token c_name = m_stack->pop();
        
        stmt.composite->name = c_name.identifier.string;
        switch(c_type.type) {
            case TK_STRUCT: { stmt.composite->type = COMP_TYPE_STRUCT; break; }
            case TK_UNION:  { stmt.composite->type = COMP_TYPE_UNION;  break; }
            default:
            {
                status = false;
                error("invalid composite structure type\n");
                break;
            }
        }
    }
    else
    {
        stmt.type = STMT_COMP_DEF;
        status = parse_composite(&stmt.composite);
    }

    if(status && (m_stack->pop().type != TK_SEMICOLON)) {
        status = false;
        error("expected ';'\n");
    }

    if(status)
    {
        stmt_list.insert(new Statement(stmt));
    }

    return status;
}

bool Parser::parse_enumerator_definition(list& stmt_list)
{
    bool status = true;
    Statement stmt = {};

    if((m_stack->peek(1).type == TK_IDENTIFIER) && (m_stack->peek(2).type == TK_SEMICOLON))
    {
        stmt.type = STMT_ENUM_DECL;
        stmt.enumerator = new Enumerator();

        m_stack->pop(); // consume the 'enum' token
        Token e_name = m_stack->pop();
        stmt.enumerator->name = e_name.identifier.string;
    }
    else
    {
        stmt.type = STMT_ENUM_DEF;
        status = parse_enumerator(&stmt.enumerator);
    }

    if(status && (m_stack->pop().type != TK_SEMICOLON)) {
        status = false;
        error("expected ';'\n");
    }

    if(status)
    {
        stmt_list.insert(new Statement(stmt));
    }

    return status;
}

bool Parser::parse_declaration(list& stmt_list)
{
    bool status = true;

    Variable* type = nullptr;
    Variable* var  = nullptr;
    strptr name = {};
    
    status = parse_variable(&type);
    
    var = type;
    if(status && (m_stack->peek(0).type == TK_ASTERISK)) {
        status = parse_pointer(&var);
    }

    Token tk = m_stack->pop();
    if(tk.type == TK_IDENTIFIER) {
        name = tk.identifier.string;
    } else {
        status = false;
        error("expected identifier\n");
    }

    if(m_stack->peek(0).type == TK_OPEN_SQUARE_BRACKET) {
        status = parse_array(&var);
    }

    tk = m_stack->peek(0);
    if(tk.type == TK_OPEN_ROUND_BRACKET)
    {
        status = parse_function_declaration(stmt_list, var, name);
    }
    else
    {
        status = parse_variable_declaration(stmt_list, type, var, name);
    }

    return status;
}

bool Parser::parse_function_declaration(list& stmt_list, Variable* var, strptr name)
{
    bool status = true;
    Function* func = new Function();
    func->name = name;
    func->ret_type = var;

    if(m_stack->pop().type != TK_OPEN_ROUND_BRACKET) {
        status = false;
        error("expected '('\n");
    }

    if(m_stack->peek(0).type == TK_CLOSE_ROUND_BRACKET)
    {
        m_stack->pop();
    }
    else
    {
        while(status)
        {
            Parameter* param = nullptr;
            status = parse_parameter(&param);

            if(status)
            {
                func->parameters.insert(param);
            }

            if(status)
            {
                Token tk = m_stack->pop();
                if(tk.type == TK_CLOSE_ROUND_BRACKET) {
                    break;
                } else if(tk.type != TK_COMMA) {
                    status = false;
                    error("expected ',' or ')'\n");
                }
            }
        }
    }

    uint8_t stmt_type = STMT_INVALID;
    if(status)
    {
        switch(m_stack->peek(0).type)
        {
            case TK_SEMICOLON:
            {
                m_stack->pop();
                stmt_type = STMT_FUNCTION_DECL;
                break;
            }
            case TK_OPEN_CURLY_BRACKET:
            {
                stmt_type = STMT_FUNCTION_DEF;
                status = parse_body(func->body);
                break;
            }
        }
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = stmt_type;
        stmt->function = func;

        stmt_list.insert(stmt);
    }

    return status;
}

bool Parser::parse_variable_declaration(list& stmt_list, Variable* type, Variable* var, strptr name)
{
    bool status = true;

    while(status)
    {
        Expression* value = nullptr;
        if(m_stack->peek(0).type == TK_EQUAL)
        {
            m_stack->pop();
            status = parse_expression(&value);
        }

        if(status)
        {
            Statement* stmt = new Statement();
            stmt->type = STMT_VARIABLE_DECL;
            stmt->variable_decl.name = name;
            stmt->variable_decl.type = var;
            stmt->variable_decl.value = value;

            stmt_list.insert(stmt);
        }

        if(status)
        {
            Token tk = m_stack->pop();
            if(tk.type == TK_SEMICOLON)
            {
                break;
            }
            else if(tk.type == TK_COMMA)
            {
                var = type;
                status = parse_pointer(&var);

                if(status)
                {
                    tk = m_stack->pop();
                    if(tk.type == TK_IDENTIFIER)
                    {
                        name = tk.identifier.string;
                    }
                    else
                    {
                        status = false;
                        error("expected identifier\n");
                    }
                }

                if(status && (m_stack->peek(0).type == TK_OPEN_SQUARE_BRACKET)) {
                    status = parse_array(&var);
                }
            }
            else
            {
                status = false;
                error("expected ',' or ';'\n");
            }
        }
    }

    return status;
}

bool Parser::parse_namespace(list& stmt_list)
{
    bool status = true;
    Statement* stmt = new Statement();
    stmt->type = STMT_NAMESPACE;

    Token tk = m_stack->pop();
    if(tk.type != TK_NAMESPACE)
    {
        status = false;
        error("expected 'namespace'\n");
    }

    if(status && (m_stack->peek(0).type == TK_IDENTIFIER))
    {
        tk = m_stack->pop();
        stmt->name_space.name = tk.identifier.string;
    }

    if(status) {
        status = parse_body(stmt->name_space.statements);
    }

    if(status) {
        stmt_list.insert(stmt);
    }

    return status;
}

bool Parser::parse_if_statement(Statement** ptr)
{
    bool status = true;

    if(m_stack->pop().type != TK_IF) {
        status = false;
        error("expected 'if'\n");
    }

    if(m_stack->pop().type != TK_OPEN_ROUND_BRACKET) {
        status = false;
        error("expected '('\n");
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_IF;
        
        status = parse_expression(stmt->if_stmt.body);
        if(status && (m_stack->pop().type != TK_CLOSE_ROUND_BRACKET)) {
            status = false;
            error("expected ')'\n");
        }

        if(status)
        {
            *ptr = stmt;
        }
    }
    
    return status;
}

bool Parser::parse_if_statement(list& stmt_list)
{
    bool status = true;

    Statement *head = nullptr, *tail = nullptr;
    while(status)
    {
        Statement* stmt = nullptr;
        status = parse_if_statement(&stmt);

        if(head == nullptr) { head = stmt;                    }
        else                { tail->if_stmt.else_stmt = stmt; }
        tail = stmt;

        Token tk = m_stack->peek(0);
        if(tk.type == TK_ELSE)
        {
            m_stack->pop();
            if(m_stack->peek(0).type == TK_IF)
            {
                continue;
            }
            else if(tk.type == TK_OPEN_CURLY_BRACKET)
            {
                stmt = new Statement();
                stmt->type = STMT_ELSE;
                
                status = parse_body(stmt->else_stmt.body);
                if(status)
                {
                    tail->if_stmt.else_stmt = stmt;
                }
                break;
            }
            else
            {
                status = false;
                error("unexpected token\n");
            }
        }
        else
        {
            break;
        }
    }

    return status;
}

bool Parser::parse_return_statement(list& stmt_list)
{
    bool status = true;

    if(m_stack->pop().type != TK_RETURN) {
        status = false;
        error("expected 'return'\n");
    }

    if(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_RET;
        
        status = parse_expression(&stmt->ret_stmt.expression);
        if(status && (m_stack->pop().type != TK_SEMICOLON)) {
            status = false;
            error("expected ';'\n");
        }

        if(status)
        {
            stmt_list.insert(stmt);
        }
    }

    return status;
}

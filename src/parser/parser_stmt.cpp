#include <parser.hpp>

#include <debug.hpp>
#include <symbol_table.hpp>

bool Parser::parse_body(list& stmt_list)
{
    bool status = true;

    Token tk = {};
    while(status)
    {
        tk = m_stack->peek(0);
        if(tk.type == '}') { break; }
        else
        {
            if(!parse_statement(stmt_list))
            {
                status = false;
            }
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
        case TK_NAMESPACE: { break; }
        case TK_GOTO:      { break; }
        case TK_RETURN:    { break; }
        case TK_BREAK:     { break; }
        case TK_SWITCH:    { break; }
        case TK_IF:        { break; }
        case TK_ELSE:      { break; }
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
            error("unexpected token %hhu\n", tk.type);
            break;
        }
    }

    return status;
}

bool Parser::parse_definition(list& stmt_list)
{
    bool status = true;
    
    uint8_t type = m_stack->peek(0).type;
    switch(type)
    {
        case TK_TYPE: case TK_CONST: case TK_IDENTIFIER:
        {
            status = parse_declaration(stmt_list);
            break;
        }

        case TK_STRUCT: case TK_UNION: case TK_ENUM:
        {
            if(m_stack->peek(1).type == TK_IDENTIFIER) {
                status = (type == TK_ENUM) ? parse_enumerator_definition(stmt_list) : parse_composite_definition(stmt_list);
            } else {
                status = parse_declaration(stmt_list);
            }
            break;
        }

        default: { status = false; break; }
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
        Token c_type = m_stack->pop();
        Token c_name = m_stack->pop();
        
        stmt.comp_decl.name = c_name.identifier.string;
        switch(c_type.type) {
            case TK_STRUCT: { stmt.comp_decl.type = COMP_TYPE_STRUCT; break; }
            case TK_UNION:  { stmt.comp_decl.type = COMP_TYPE_UNION;  break; }
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
        status = parse_composite(&stmt.comp_def);
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
        Token e_type = m_stack->pop();
        Token e_name = m_stack->pop();
        stmt.enum_decl.name = e_name.identifier.string;
    }
    else
    {
        stmt.type = STMT_ENUM_DEF;
        status = parse_enumerator(&stmt.enum_def);
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
        status = parse_pointer_array(&var);
    }

    Token tk = m_stack->pop();
    if(tk.type == TK_IDENTIFIER) {
        name = tk.identifier.string;
    } else {
        status = false;
        error("expected identifier\n");
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

    if(m_stack->pop().type != TK_OPEN_ROUND_BRACKET) {
        status = false;
        error("expected '('\n");
    }

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
                error("expected ',' or ';'\n");
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
            else if(tk.type == ',')
            {
                var = type;
                status = parse_pointer_array(&var);

                if(status)
                {
                    tk = m_stack->pop();
                    if(tk.type == TK_IDENTIFIER) {
                        name = tk.identifier.string;
                    } else {
                        status = false;
                        error("expected identifier\n");
                    }
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

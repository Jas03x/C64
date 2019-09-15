#include <parser.hpp>

#include <debug.hpp>
#include <symbol_table.hpp>

bool Parser::parse_body(list& statements)
{
    bool status = true;

    Token tk = {};
    while(status)
    {
        tk = m_stack->peek(0);
        if(tk.type == '}') { break; }
        else
        {
            Statement* stmt = nullptr;
            if(parse_statement(statements))
            {
                if(stmt != nullptr) {
                    statements.insert(stmt);
                }
            } else {
                status = false;
            }
        }
    }

    return status;
}

bool Parser::parse_statement(list& statements)
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
        case TK_OPEN_CURLY_BRACKET: { break; }

        case TK_TYPEDEF:
        case TK_UNION: case TK_STRUCT: case TK_ENUM:
        case TK_TYPE:  case TK_CONST:  case TK_EXTERN:
        {
            status = parse_definition(statements);
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
                    status = parse_definition(statements);
                    break;
                }

                default:
                {
                    status = parse_expression(statements);
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

bool Parser::parse_definition(list& statements)
{
    bool status = true;
    
    uint8_t type = m_stack->peek(0).type;
    switch(type)
    {
        case TK_TYPEDEF: { status = parse_typedef(statements); break; }

        case TK_TYPE: case TK_CONST: case TK_IDENTIFIER:
        {
            status = parse_variable_definition(statements);
            break;
        }

        case TK_STRUCT: case TK_UNION: case TK_ENUM:
        {
            if(m_stack->peek(1).type == TK_IDENTIFIER) {
                status = (type == TK_ENUM) ? parse_enumerator_definition(statements) : parse_composite_definition(statements);
            } else {
                status = parse_variable_definition(statements);
            }
            break;
        }

        default: { status = false; break; }
    }

    return status;
}

bool Parser::parse_composite_definition(list& statements)
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
        statements.insert(new Statement(stmt));
    }

    return status;
}

bool Parser::parse_enumerator_definition(list& statements)
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
        statements.insert(new Statement(stmt));
    }

    return status;
}

bool Parser::parse_variable_definition(list& statements)
{
    bool status = true;

    Variable* type = nullptr;
    status = parse_variable(&type);

    while(status)
    {
        Statement* stmt = new Statement();
        stmt->type = STMT_VARIABLE_DECL;
        stmt->variable_decl.type = type;

        Token tk = m_stack->pop();
        if(tk.type == TK_IDENTIFIER) {
            stmt->variable_decl.name = tk.identifier.string;
        } else {
            status = false;
            error("expected identifier\n");
        }

        if(status) {
            status = parse_pointer_array(&stmt->variable_decl.type);
        }

        if(status && (m_stack->peek(0).type == TK_EQUAL)) {
            status = parse_expression(&stmt->variable_decl.value);
        }

        if(status)
        {
            statements.insert(stmt);

            tk = m_stack->pop();
            if(tk.type == TK_SEMICOLON) {
                break;
            } else if(tk.type != TK_COMMA) {
                status = false;
                error("expected ',' or ';'\n");
            }
        }
    }

    return status;
}

#include <parser.hpp>

#include <debug.hpp>
#include <symbol_table.hpp>

bool Parser::parse_variable(Variable** ptr)
{
    bool status = true;

    Variable* var = new Variable();
    status = parse_variable_modifiers(var->flags);

    if(status)
    {
        Token tk = m_stack->peek(0);
        switch(tk.type)
        {
            case TK_TYPE:
            {
                m_stack->pop();
                var->type = tk.subtype;
                break;
            }

            case TK_IDENTIFIER:
            {
                Identifier* identifier = nullptr;
                status = parse_identifier(&identifier);

                if(status)
                {
                    var->type = TYPE_UNKNOWN;
                    var->identifier = identifier;
                }

                break;
            }

            case TK_UNION: case TK_STRUCT:
            {
                Composite* composite = nullptr;
                status = parse_composite(&composite);

                if(status && (composite->name.len != 0)) {
                    status = false;
                    error("inline composite cannot have a name\n");
                }

                if(status)
                {
                    var->type = TYPE_COMPOSITE;
                    var->composite = composite;
                }

                break;
            }

            case TK_ENUM:
            {
                Enumerator* enumerator = nullptr;
                status = parse_enumerator(&enumerator);

                if(status && (enumerator->name.len != 0)) {
                    status = false;
                    error("inline enumerator cannot have a name\n");
                }

                if(status)
                {
                    var->type = TYPE_ENUMERATOR;
                    var->enumerator = enumerator;
                }

                break;
            }

            default:
            {
                status = false;
                error("unexpected token %s\n", token_to_str(tk));
                break;
            }
        } // switch
    } // if

    if(status)
    {
        *ptr = var;
    }

    return status;
}

bool Parser::parse_variable_modifiers(VariableFlags& flags)
{
    bool status = true, running = true;
    while(status && running)
    {
        switch(m_stack->peek(0).type)
        {
            case TK_CONST:
            {
                m_stack->pop();
                if(flags.is_constant != 1) { flags.is_constant = 1; }
                else { status = false; error("duplicated 'const' modifier\n"); }
                break;
            }

            case TK_EXTERN:
            {
                m_stack->pop();
                if(flags.is_external_symbol != 1) { flags.is_external_symbol = 1; }
                else { status = false; error("duplicated 'extern' modifier\n"); }
                break;
            }

            default:
            {
                running = false;
            }
        }
    }

    return status;
}

bool Parser::parse_composite(Composite** ptr)
{
    bool status = true;
    Composite* composite = new Composite();

    Token tk = m_stack->pop();
    switch(tk.type)
    {
        case TK_STRUCT: { composite->type = COMP_TYPE_STRUCT; break; }
        case TK_UNION:  { composite->type = COMP_TYPE_UNION;  break; }
        default: { status = false; error("Expected 'struct' or 'union'\n"); }
    }

    if (status && (m_stack->peek(0).type == TK_IDENTIFIER))
	{
        tk = m_stack->pop();
        composite->name = tk.identifier.string;
	}

    if(status)
    {
        status = parse_body(composite->body);
    }

    if(status)
    {
        *ptr = composite;
    }

    return status;
}

bool Parser::parse_enumerator(Enumerator** ptr)
{
    bool status = true;
    Enumerator* enumerator = new Enumerator();

    Token tk = m_stack->pop();
    if(tk.type != TK_ENUM)
    {
        status = false;
        error("expected 'enum'\n");
    }

    if(status && (m_stack->peek(0).type == TK_IDENTIFIER))
    {
        tk = m_stack->pop();
        enumerator->name = tk.identifier.string;
    }

    if(status && (m_stack->pop().type != TK_OPEN_CURLY_BRACKET))
    {
        status = false;
        error("expected '{'\n");
    }

    if(status)
    {
        if(m_stack->peek(0).type == TK_CLOSE_CURLY_BRACKET)
        {
            m_stack->pop();
        }
        else
        {
            while(status)
            {
                Enumerator::Value* value = new Enumerator::Value();
                
                tk = m_stack->pop();
                if(tk.type == TK_IDENTIFIER) {
                    value->name = tk.identifier.string;
                } else {
                    status = false;
                    error("expected identifier\n");
                }

                if(status && (m_stack->peek(0).type == TK_EQUAL))
                {
                    m_stack->pop();
                    status = parse_expression(&value->value);
                }
                
                if(status)
                {
                    enumerator->values.insert(value);

                    tk = m_stack->pop();
                    if(tk.type == TK_CLOSE_CURLY_BRACKET)
                    {
                        break;
                    }
                    else if(tk.type != TK_COMMA)
                    {
                        status = false;
                        error("expected ',' or '}'\n");
                    }
                }
            }
        }
    }

    if(status)
    {
        *ptr = enumerator;
    }

    return status;
}

bool Parser::parse_pointer(Variable** ptr)
{
    bool status = true;

    Variable* head = *ptr;
    while(m_stack->peek(0).type == TK_ASTERISK)
    {
        m_stack->pop();

        Variable* var = new Variable();
        var->type = TYPE_PTR;
        var->flags.value = head->flags.value;
        var->pointer = head;

        head = var;
    }

    if(status)
    {
        *ptr = head;
    }

    return status;
}

bool Parser::parse_array(Variable** ptr)
{
    bool status = true;

    Variable* head = *ptr;
    while(m_stack->peek(0).type == TK_OPEN_SQUARE_BRACKET)
    {
        m_stack->pop();

        Variable* var = new Variable();
        var->flags.value = head->flags.value;
        var->array.elements = head;

        if(m_stack->peek(0).type == TK_CLOSE_SQUARE_BRACKET)
        {
            var->type = TYPE_VARIABLE_SIZED_ARRAY;
        }
        else
        {
            var->type = TYPE_CONSTANT_SIZED_ARRAY;
            status = parse_expression(&var->array.size);
        }

        if(m_stack->pop().type != TK_CLOSE_SQUARE_BRACKET)
        {
            status = false;
            error("expected ']'\n");
        }
    }

    return status;
}

bool Parser::parse_parameter(Parameter** ptr)
{
    bool status = true;

    Parameter* param = new Parameter();
    status = parse_variable(&param->type);

    if(status && (m_stack->peek(0).type == TK_ASTERISK)) {
        status = parse_pointer(&param->type);
    }

    if(status && (m_stack->peek(0).type == TK_IDENTIFIER))
    {
        Token tk = m_stack->pop();
        param->name = tk.identifier.string;
    }

    if(status && (m_stack->peek(0).type == TK_OPEN_SQUARE_BRACKET)) {
        status = parse_array(&param->type);
    }

    if(status) {
        *ptr = param;
    }

    return status;
}

bool Parser::parse_identifier(Identifier** ptr)
{
    bool status = true;
    Identifier *head = nullptr, *tail = nullptr;

    while(status)
    {
        Token tk = m_stack->pop();
        if(tk.type == TK_IDENTIFIER)
        {
            Identifier* id = new Identifier();
            id->str = tk.identifier.string;

            if(tail == nullptr) { head = id;       }
            else                { tail->next = id; }
            tail = id;

            Token tk0 = m_stack->peek(0);
            Token tk1 = m_stack->peek(1);
            if((tk0.type == TK_COLON) && (tk1.type == TK_COLON))
            {
                m_stack->pop();
                m_stack->pop();
            }
            else
            {
                break;
            }
        }
        else
        {
            status = false;
            error("expected identifier\n");
        }
    }

    if(status)
    {
        *ptr = head;
    }

    return status;
}

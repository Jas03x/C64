
#include "token.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

#include <parser.hpp>

#include <debug.hpp>

bool process(const char* path)
{
	TokenStack token_stack;
    bool status = Tokenizer::Tokenize(path, &token_stack);

    const std::vector<Token>& tokens = token_stack.get_tokens();
    for(unsigned int i = 0; i < tokens.size(); i++)
    {
        print_token(tokens[i]);
    }

    if(status)
    {
        AST* ast = Parser::Parse(token_stack);
        if(ast != nullptr)
        {
            print_ast(*ast);
        }
        else
        {
            status = false;
            error("an error occured while parsing\n");
        }
        
    }

    return status;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Invalid arguments\n");
        return -1;
    }

    if(!process(argv[1]))
    {
        return -1;
    }

    printf("Terminating\n");
    return 0;
}

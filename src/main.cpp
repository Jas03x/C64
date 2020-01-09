
#include "token.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

#include <parser.hpp>

#include <debug.hpp>

bool process(const char* path)
{
	TokenStack tokens;
    
    bool status = Tokenizer::Tokenize(path, &tokens);
    /*
    if(status)
    {
        AST* ast = Parser::Parse(&tokens);
        if(ast != nullptr)
        {
            print_ast(ast);
        }
        else
        {
            status = false;
            error("an error occured while parsing\n");
        }
        
    }
    */

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

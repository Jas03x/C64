
#include "token.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

#include <parser.hpp>

#include <debug.hpp>

bool process(const char* path)
{
    bool status = true;

	TokenStack tokens;    
    if(Tokenizer::Process(path, tokens))
    {
        AST* ast = Parser::Parse(&tokens);
        if(ast != nullptr)
        {
            printf("Successfully parsed!\n");
            debug_print_ast(ast);
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

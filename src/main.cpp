
#include "token.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

#include <file.hpp>
#include <parser.hpp>

bool process(const char* path)
{
    bool status = true;

    unsigned int source_size = 0;
    char* source_code = File::Read(path, source_size);

    if(source_code != nullptr)
    {
        Tokenizer tokenizer = Tokenizer(source_code, source_size);
        
        if(tokenizer.tokenize())
        {
            Parser parser = Parser(&tokenizer.get_tokens());
            if(parser.process())
            {
            }
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

    return 0;
}


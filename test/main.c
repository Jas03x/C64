
VOID print(const U8* buffer);
/*
namespace io
{
    VOID print(const U8* buffer);
}
*/

struct Person
{
    const U8* name;
    U32       age;
}

U32 main(U32 argc, U8* argv[])
{
    Person jas = { "Jas", 21 };

    if(true)
    {
        struct out_of_scope
        {
            U64 var;
        }

        out_of_scope test1;
    }

    out_of_scope test2;

    print(jas.name);
    //io::print(jas.name);

    return 0;
}

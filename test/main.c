
VOID printf(const U8* buffer);

struct Person
{
    const U8* name;
    U32       age;
}

U32 main(U32 argc, U8* argv[])
{
    Person jas = { "Jas", 21 };

    while(true)
    {
        U8 i = 0;
        for(; i < 10;)
        {
            printf("ding a dong\n");
            i = i + 1;
        }
    }

    return 0;
}


VOID printf(const U8* buffer);

U32 main(U32 argc, U8* argv[])
{
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

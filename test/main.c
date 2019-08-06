
VOID printf(const U8* buffer);

U32 main()
{
    while(true)
    {
        for(U8 i = 0; i < 10; i = i + 1)
        {
            printf("ding a dong\n");
        }
    }

    return 0;
}

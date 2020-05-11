
void print(const U8*);
void print_int(U32);

U32 main(U32 argc, const U8** argv)
{
    for(U32 i = 0; i < 10; i++)
    {
        print("hello world\n");
    }
    return 0;
}

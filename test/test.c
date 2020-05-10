
void print(const U8*);
void print_int(U32);

U32 main(U32 argc, const U8** argv)
{
    print("hello world\n");
    print_int(1 + 2 * 3 + 4 * 5 * 6 + 7);
    return 0;
}

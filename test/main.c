
U32 foo(U32 val)
{
    return val + 1;
}

U32 fub(U32 v0, U32 v1)
{
    return v1 + 1 - v0 + 1;
}

U32 global_integer = 3;

U32 main()
{
    U32 a = 1 * 2 + 3 * 4 * 5;
    U32 b = 4 + 1;
    U32 c = a * a + b + b;

    if(100 < a * b + c)
    {
        print("HELLO WORLD\n");
    }

    *(a + (2 + 2)) = b + 1;

    return fub(foo(a) * *(b + c), b + foo(c));
}

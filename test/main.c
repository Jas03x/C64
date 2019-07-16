
U32 foo(U32 val)
{
    return val + 1;
}

U32 main()
{
	U32 a = 1 * 2 + 3 * 4 * 5;
    U32 b = 4 + 1;
    U32 c = a * a + b + b;
    
    return foo(foo(a) * b + foo(c));
}

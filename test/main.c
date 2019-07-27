
void printf(const U8* buffer);

struct Person
{
    const U8* name;
    U32       age;

    void print_hi()
    {
        printf(name);
    }
}

U32 main()
{
    Person jas;
    jas.name = "Jas";
    jas.age  = 21;

    jas.print_hi();

    return 0;
}

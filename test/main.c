
/*
struct Person
{
    const U8* name;
    U32       age;
}
*/

extern const U8*[2] counter;

void sprintf(U8* buffer, const U8* format, const U8* param);
void printf(const U8* buffer);

U32 main()
{
    U8* [8][16][24][32] unused;

    const U8* format = "Hi my name is %s\n";
    const U8* name   = "Jas";

    if(1 < 2)
    {
        printf("lmao no optimizations\n");
    }

    U8[256] buffer;
    sprintf(buffer, format, name);

    printf(buffer);

    return 0;
}

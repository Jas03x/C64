
U32 main(U32 argc, const U8** argv)
{
    U8 i = 1 + 2 * 3 / 4;
    if(i < 10)
        print("yes");
    else if (i == 10)
        print("yesn't");
    else
        print("no");
    return 0;
}


namespace io
{
    VOID print(const U8* buffer);
}

U32 main(U32 argc, U8* argv[])
{
	for(;;)
	{
		io::print("hello world");
	}

    return 0;
}

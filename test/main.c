
namespace io
{
    VOID print(const U8* buffer);
	U8   get_char();

	typedef struct { VOID* handle; } FILE;

	namespace file
	{
		FILE* open(const U8* path);
		VOID  close(FILE* handle);
	}
}

U32 main(U32 argc, U8* argv[])
{
	switch(io::get_char())
	{
		case 'A': io::print("very good!\n"); break;
		case 'B': io::print("not bad!\n");   break;
		default:
		{
			io::print("you are a failure\n");
			break;
		}
	}

    return 0;
}

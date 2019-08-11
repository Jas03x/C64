
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
	VOID* file = io::file::open("file.txt");

	char* str = "\x48\x45\x4C\x4C\x4F\x20WORLD";

	U8 running = 1;
	while(running == 1)
	{
		U8 input = io::get_char();

		if(input == 'A')
		{
		}
	}

	io::file::close(file);

    return 0;
}

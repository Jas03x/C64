
namespace io
{
    VOID print(const U8* buffer);

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

	for(;;)
	{
		io::print("hello world");
	}

	io::file::close(file);

    return 0;
}

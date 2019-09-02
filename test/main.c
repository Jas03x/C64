
#include <io.h>

struct Person
{
	U8  age;
	U8* name;

	F32 height;

	void print()
	{
		io::print("name = %s\n", name);
		io::print("age = %u\n",  age);
		io::print("height = %f\n", height);
	}
};

U32 main(U32 argc, U8* argv[])
{
	Person jas = { 21, "Jas", 6.0 };

    return 0;
}

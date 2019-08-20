
#include <io.h>

// #define print_str(str) io::print(str)

struct Person
{
	U8  age;
	U8* name;
};

void* jas;

U32 main(U32 argc, U8* argv[])
{
	jas = new_person();
	// ((Person*) jas)->age = 0;

	print_str("Hello world\n");

    return 0;
}

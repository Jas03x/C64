
import io;

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

	io::print("Hello world\n");

    return 0;
}


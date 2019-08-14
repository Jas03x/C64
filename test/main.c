
import io;

VOID run(VOID(*pfn_array[2])(U8))
{
	pfn_array[0](1);
	pfn_array[1](1);
}

VOID print_hello(U8 val)
{
	if(val != 0)
	{
		io::print("hello ");
	}
}

VOID print_world(U8 val)
{
	if(val != 0)
	{
		io::print("world\n");
	}
}

typedef VOID (*pfn_print_hello_world[2])();

U32 main(U32 argc, U8* argv[])
{
	VOID (*pfn_array[2])() =
	{
		&print_hello,
		&print_world
	};

	run(pfn_array);

    return 0;
}


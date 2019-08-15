
import io;

U32 main(U32 argc, U8* argv[])
{
	F64 a = 3.1452;
	//U64 b = static_cast<U64>(a);
	//I64 c = reinterpret_cast<I64>(a);
	U64 b = (U64) a;

	void* strt = (const struct { U64 a; U32 b; }*) { 1, 2 };
	
	io::print("Hello world\n");

    return 0;
}


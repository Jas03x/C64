
U32 main(U32 argc, U8* argv[])
{
	F64 a = 3.1452;
	U64 b = cast<U64>(a);
	I64 c = r_cast<I64>(a);
	U64 d = (U64) b;

	void* strt = (const struct { U64 a; U32 b; }*) { 1, 2 };
	
	io::print("Hello world\n");

    return 0;
}


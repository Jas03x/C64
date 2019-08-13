
import io;

union var
{
	U8  u8;
	U16 u6;
	U32 u32;
	U64 u64;

	I8  i8;
	I16 i6;
	I32 i32;
	I64 i64;

	VOID* ptr;
};

U32 main(U32 argc, U8* argv[])
{
	union
	{
		U8 a;
		U8 b;
	} pair;

	enum STATUS
	{
		BAD  = 0,
		OK   = 1,
		GOOD = 2
	};

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


import io;

U32 main(U32 argc, U8* argv[])
{
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

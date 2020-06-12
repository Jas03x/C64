
//#include <stdio.h>
//typedef int I32;
//typedef unsigned int U32;

U32 sqr(U32 i)
{
	return i * i;
}

U32 val = 2;
U32(*val_ptr())
{
	return &val;
}

U32(*get_pfn())(U32)
{
	return &sqr;
}

U32(*(*get_pfn_pfn())())(U32) 
{
	return &get_pfn;
}

I32 main()
{
	U32(*pfn_sqr)(U32) = &sqr;

	U32(*(*(*pfn)())())(U32) = &get_pfn_pfn;
	U32(*(*(**ppfn)())())(U32) = &pfn;

	printf("%u\n", (*pfn)()()(*val_ptr()));
	return 0;
}

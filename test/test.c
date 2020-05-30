
//#include <stdio.h>
//typedef unsigned int U32;

U32 sqr(U32 i)
{
	return i * i;
}

U32 (*get_pfn(void))(U32)
{
	return sqr;
}

int main(void)
{
	U32(*pfn_sqr)(U32) = &sqr;
	U32(**ppfn_sqr)(U32) = &pfn_sqr;

	printf("%u\n", get_pfn()(2));
	return 0;
}

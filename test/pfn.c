#include <stdio.h>

void odd()  { printf("odd!\n");  }
void even() { printf("even!\n"); }

void (*get_pfn(int i))()
{
    return i % 2 == 0 ? &even : &odd;
}

void (*(*get_pfn_pfn(void))(int))(void)
{
    return &get_pfn;
}

int main()
{
    void (*pfn_array[2])(void) = { &even, &odd };
    void (*(*pfn_get_pfn)(int))(void) = &get_pfn;
    void (*(*(*pfn_get_pfn_pfn)(void))(int))(void) = &get_pfn_pfn;

    // BAD PFN:
    void (*(*(*bad_pfn)(void))(int))(void) = (void (*(*(*)(void))(int))(void)) &even;

    get_pfn_pfn()(1)();
    get_pfn_pfn()(2)();
    return 0;
}

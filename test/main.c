
VOID printf(const U8* buffer);

U64 some_function();

struct Person
{
    const U8* name;
    U32       age;
}

U32 main()
{
    U16 array[2][2][2];

    U64** array2[4][4];

    U32 array3[some_function(123)];

    // array[x]       -> U16[2][2]
    // array[x][y]    -> U16[2]
    // array[x][y][z] -> U16
    // Pattern: The declaration works backwards! Simply flip the declaration order, then you have the correct order!
    //          For example: [2][2][2] U16, or *** U16

    Person persons[2];

    Person bob;

    Person jas;
    jas.name = "Jas";
    jas.age  = 21;

    persons[0] = jas;
    persons[1] = bob;

    return 0;
}

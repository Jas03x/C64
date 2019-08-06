
VOID printf(const U8* buffer);

struct Person
{
    const U8* name;
    U32       age;
}

U32 main()
{
    // U16*** array;
    U16[2][2][2] array;

    // array[x]       -> U16[2][2]
    // array[x][y]    -> U16[2]
    // array[x][y][z] -> U16
    // Pattern: The declaration works backwards! Simply flip the declaration order, then you have the correct order!
    //          For example: [2][2][2] U16, or *** U16

    Person[2] persons;

    Person bob;

    Person jas;
    jas.name = "Jas";
    jas.age  = 21;

    persons[0] = jas;
    persons[1] = bob;

    return 0;
}

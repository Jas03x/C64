
namespace io
{
    void print(const U8*);

    struct file
    {
        void* handle;
    };

    file* open(const U8*);
    void  close(file*);
    U32   get_size(file*);
    U32   read(file*, U32, U8*);
}

struct Person
{
    const U8* name;
    U32 age;
};

void print_person_name(Person* p)
{
    io::print("Person: ");
    io::print(p->name);
    io::print("\n");
}

void free();

U32 main(U32 argc, U8* argv[])
{
    io::print("hello world");

    io::file* file = io::open("~/test.txt");
    U32 size = io::get_size(file);
    U8* buffer = malloc(size);
    if(read(file, size, buffer) != size)
    {
        io::print("an error occured while reading the file\n");
    }
    else
    {
        io::print(buffer);
    }
    free(buffer);

    struct { const U8* name; U32 age; } jas = { "Jas", 21 };
    print_person_name(const Person* (&jas));

    return 0;
}


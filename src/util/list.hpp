#ifndef LIST_HPP
#define LIST_HPP

struct list
{
    struct element
    {
        void* ptr;
        element* next;
    };
    
    element* head;
    element* tail;
    unsigned int count;

    void free();
    void insert(void* ptr);
};

#endif // LIST_HPP
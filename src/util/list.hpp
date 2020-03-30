#ifndef LIST_HPP
#define LIST_HPP

template<typename T>
class List
{
public:
    struct Element
    {
        T* ptr;
        Element* next;
    };
    
    Element* head;
    Element* tail;
    unsigned int count;

    void free();
    void insert(T* ptr);
};

#endif // LIST_HPP
#include "list.hpp"

void list::free()
{
    for(element* it = this->head; it != nullptr;)
    {
        element* e = it;
        it = it->next;

        delete e;
    }
}

void list::insert(void* ptr)
{
    element* e = new element();
    e->ptr = ptr;

    if(this->head == nullptr) { this->head = e;       }
    else                      { this->tail->next = e; }
    this->tail = e;

    this->count ++;
}

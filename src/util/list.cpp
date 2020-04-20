#include "list.hpp"

#include <parser/ast.hpp>

template <typename T>
void List<T>::free()
{
    for(List<T>::Element* it = this->head; it != nullptr;)
    {
        List<T>::Element* e = it;
        it = it->next;

        delete e;
    }
}

template <typename T>
void List<T>::insert(T* ptr)
{
    List<T>::Element* e = new List<T>::Element();
    e->ptr = ptr;

    if(this->head == nullptr) { this->head = e;       }
    else                      { this->tail->next = e; }
    this->tail = e;

    this->count ++;
}

// instantiate the template for whatever classes use it
template class List<Statement>;
template class List<Parameter>;
template class List<Expression>;


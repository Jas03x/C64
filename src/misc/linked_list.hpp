#ifndef LINKED_LIST_HPP
#define LINKED_LIST_HPP

template <typename T>
class LList
{
private:
    T* head;
    T* tail;

public:
    LList()
    {
        head = nullptr;
        tail = nullptr;
    }

    void insert(T* t)
    {
        if(head == nullptr)
        {
            head = t;
        }
        else
        {
            tail->next = t;
        }

        tail = t;
    }

    T* get_head()
    {
        return head;
    }
};

#endif // LINKED_LIST_HPP
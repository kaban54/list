#include "list.h"


int main()
{
    List_t list = {};

    ListCtor (&list, 7);

    ListInsertTail (&list, 1);
    ListInsertTail (&list, 2);
    ListInsertTail (&list, 3);

    ListDump (&list);

    ListDtor (&list);
    
    return 0;
}
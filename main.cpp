#include "list.h"


int main()
{
    List_t list = {};

    ListCtor (&list, 7);

    ListInsertTail (&list, 1);
    ListDump (&list);
    ListInsertTail (&list, 2);
    ListDump (&list);
    ListInsertTail (&list, 3);
    ListDump (&list);
    ListInsertHead (&list, 4);
    ListDump (&list);
    ListInsertHead (&list, 5);
    ListDump (&list);

    ListLinearize (&list);
    ListDump (&list);

    ListDtor (&list);

    return 0;
}
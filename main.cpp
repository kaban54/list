#include "list.h"


int main()
{
    List_t list = {};

    ListCtor (&list, 1);

    ListInsertHead (&list, 228);    
    ListInsertTail (&list, 54);
    ListInsertVal (&list, 1, 31);

    int a = 0, b = 0, c = 0;

    ListPopVal (&list, 1, &a);
    ListPopVal (&list, 2, &b);
    ListPopVal (&list, 3, &c);

    printf ("\n%d %d %d\n", a, b, c);

    ListTxtDump (&list, stdout);

    return 0;
}
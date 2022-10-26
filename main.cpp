#include "list.h"


int main()
{
    List_t list = {};

    ListCtor (&list, 7);

    ListInsertHead (&list, 228);
    ListTxtDump (&list, stdout);    
    ListInsertTail (&list, 54);
    ListTxtDump (&list, stdout);
    ListInsertVal (&list, 1, 31);
    ListTxtDump (&list, stdout);

    ListLinearize (&list);

    ListTxtDump (&list, stdout);

    int a = 0, b = 0, c = 0;

    ListPopVal (&list, 1, &a);
    ListTxtDump (&list, stdout);
    ListPopVal (&list, 2, &b);
    ListTxtDump (&list, stdout);
    ListPopVal (&list, 3, &c);

    printf ("\n%d %d %d\n", a, b, c);

    ListTxtDump (&list, stdout);

    return 0;
}
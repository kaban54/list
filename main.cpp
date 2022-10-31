#include "list.h"

FILE *LOG = NULL;

int main()
{
    LOG = fopen (LOGFILENAME, "w");
    if (LOG == nullptr) return 0;
    fprintf (LOG, "<pre>\n");

    List_t list = {};

    ListCtor (&list, 7);

    ListInsertHead (&list, 1);
    fprintf (LOG, "<hr><h2>INSERT 1 IN HEAD</h2>");
    ListDump (&list);

    ListInsertHead (&list, 2);
    fprintf (LOG, "<hr><h2>INSERT 2 IN HEAD</h2>");
    ListDump (&list);

    ListInsertTail (&list, 3);
    fprintf (LOG, "<hr><h2>INSERT 3 IN TAIL</h2>");
    ListDump (&list);

    ListLinearize (&list);
    fprintf (LOG, "<hr><h2>LINEARIZE</h2>");
    ListDump (&list);

    ListDtor (&list);

    fclose (LOG);
    return 0;
}
#include "list.h"



int List_ctor (List_t *list, int capacity, const char *name, const char *func_name, const char *file_name, int line)
{
    if (list == nullptr) return LIST_NULLPTR_ARG;

    if (list -> status != LIST_CREATED)
    {
        list -> err |= LIST_STATUS_ERROR;
        ListPrintError (list);
        return list -> err;
    }

    list -> err |= ListSetInfo (list, name, func_name, file_name, line);

    list -> err |= ListConstructData (list, capacity);

    list -> happy  = 1;
    list -> shift  = 0;
    list -> status = LIST_CONSTRUCTED;

    ListVerify (list);

    return LIST_OK;
}

int ListSetInfo (List_t *list, const char *name, const char *func_name, const char *file_name, int line)
{
    if (list == nullptr) return LIST_NULLPTR_ARG;

    list -> info.     name =      name;
    list -> info.func_name = func_name;
    list -> info.file_name = file_name;
    list -> info.     line =      line;

    return LIST_OK;
}

int ListConstructData (List_t *list, int capacity)
{
    if (list == nullptr) return LIST_NULLPTR_ARG;
    if (capacity < 0)    return LIST_INCORRECT_CAPACITY;

    list -> data = (ListElem_t *) (calloc (capacity + 1, sizeof (list -> data [0])));
    if (list -> data == nullptr)
    {
        list -> err |= LIST_ALLOC_ERROR;
        return list -> err;
    }

    list -> capacity = capacity;

    for (int index = 0; index <= capacity; index++)
    {
        list -> data [index].value = POISON_VAL;
        list -> data [index].next  = index + 1;
        list -> data [index].prev  = POISON_INDEX;
    }
    list -> data [capacity].next = 0;

    list -> free = list -> data [0].next;

    list -> data [0].prev = 0;
    list -> data [0].next = 0;

    return LIST_OK;
}


int ListDtor (List_t *list)
{
    ListVerify (list);

    free (list -> data);

    list -> data = nullptr;
    list -> capacity = 0;
    list -> free = POISON_INDEX;
    
    list -> status = LIST_DECONSTRUCTED;

    return LIST_OK;
}


int ListGetHead (List_t *list, int *head)
{
    ListVerify (list);
    if (head == nullptr) return LIST_NULLPTR_ARG;

    *head = list -> data [0].next;
    return LIST_OK;
}

int ListGetTail (List_t *list, int *tail)
{
    ListVerify (list);
    if (tail == nullptr) return LIST_NULLPTR_ARG;

    *tail = list -> data [0].prev;
    return LIST_OK;
}


int ListInsertVal (List_t *list, int prev, val_t value)
{
    ListVerify (list);

    if (prev < 0 || prev > list -> capacity || list -> data [prev].prev == POISON_INDEX)
    {
        list -> err = LIST_INCORRECT_INDEX;
        ListPrintError (list);
        return list -> err;
    }

    if (!list -> free)
    {
        if (ListExpand (list, list -> capacity == 0 ? LIST_BASE_CAPACITY : list -> capacity * 2 + 1)) return list -> err;
    }

    int index = list -> free;
    int next  = list -> data [prev].next;

    list -> free = list -> data [index].next;

    list -> data [index].prev  = prev;
    list -> data [index].next  = next;
    list -> data [index].value = value;

    list -> data [next].prev = index;
    list -> data [prev].next = index;

    if (list -> happy)
    {
        if      (prev == 0 && next == index + 1             ) list -> shift -= 1;
        else if (prev == 0 && next == 0 && list -> shift > 0) list -> shift -= 1;
        else if (next != 0)                                   list -> happy  = 0;
    }

    ListVerify (list);
    return LIST_OK;
}

int ListInsertHead (List_t *list, val_t value)
{
    ListVerify (list);
    return ListInsertVal (list, 0, value);
}

int ListInsertTail (List_t *list, val_t value)
{   
    ListVerify (list);
    return ListInsertVal (list, list -> data[0].prev, value);
}

int ListPopVal (List_t *list, int index, val_t *value)
{
    ListVerify (list);

    if (index <= 0 || index > list -> capacity || list -> data [index].prev == POISON_INDEX)
    {
        list -> err = LIST_INCORRECT_INDEX;
        ListPrintError (list);
        return list -> err;
    }

    *value = list -> data [index].value;

    int prev = list -> data [index].prev;
    int next = list -> data [index].next;

    list -> data [prev].next = next;
    list -> data [next].prev = prev;

    list -> data [index].prev  = POISON_INDEX;
    list -> data [index].value = POISON_VAL;
    list -> data [index].next = list -> free;
    list -> free = index;

    if (list -> happy)
    {
        if      (prev == 0) list -> shift += 1;
        else if (next != 0) list -> happy  = 0;
    }

    ListVerify (list);
    return LIST_OK;
}


int ListGetIndex (List_t *list, int position, int *index)
{
    ListVerify (list);

    if (index == nullptr) return LIST_NULLPTR_ARG;

    if (position <= 0 || position > list -> capacity)
    {
        list -> err = LIST_INCORRECT_INDEX;
        ListPrintError (list);
        return list -> err;
    }

    if (list -> happy)
    {
        if (position + list -> shift > list -> capacity)
        {
            list -> err = LIST_INCORRECT_INDEX;
            ListPrintError (list);
            return list -> err;
        }
        *index = position + list -> shift;
    }
    else
    {
        int ind = list -> data [0].next;
        for (position--; position > 0; position--)
        {
            if (ind == 0)
            {
                list -> err = LIST_INCORRECT_INDEX;
                ListPrintError (list);
                return list -> err;
            }
            ind = list -> data [ind].next;
        }
        *index = ind;
    }

    ListVerify (list);
    return LIST_OK;
}

int ListLinearize (List_t *list)
{
    ListVerify (list);

    ListElem_t *new_data = (ListElem_t *) calloc (list -> capacity + 1, sizeof (list -> data[0]));
    if (new_data == nullptr)
    {
        list -> err |= LIST_ALLOC_ERROR;
        return list -> err;
    }

    int index_old = 0;
    int index_new = 0;

    for (;index_new <= list -> capacity; index_new++)
    {
        new_data [index_new].next  = index_new + 1;
        new_data [index_new].prev  = index_new - 1;
        new_data [index_new].value = list -> data [index_old].value;

        index_old = list -> data [index_old].next;
        if (index_old == 0) break;
    }

    new_data [0]        .prev = index_new;
    new_data [index_new].next = 0;

    list -> free = index_new + 1;

    for (index_new++; index_new <= list -> capacity; index_new++)
    {
        new_data [index_new].next  = index_new + 1;
        new_data [index_new].prev  = POISON_INDEX;
        new_data [index_new].value = POISON_VAL;
    }

    new_data [list -> capacity].next = 0;

    free (list -> data);
    list -> data = new_data;

    list -> happy = 1;
    list -> shift = 0;

    return LIST_OK;    
}

int ListExpand (List_t *list, int capacity)
{
    ListVerify (list);

    if (capacity < list -> capacity)
    {
        list -> err = LIST_INCORRECT_CAPACITY;
        ListPrintError (list);
        return list -> err;
    }

    list -> data = (ListElem_t *) Recalloc (list -> data, capacity + 1, sizeof (list -> data [0]), list -> capacity + 1);
    if (list -> data == nullptr)
    {
        list -> err = LIST_ALLOC_ERROR;
        ListPrintError (list);
        return list -> err;
    }

    for (int index = list -> capacity + 1; index <= capacity; index++)
    {
        list -> data [index].value = POISON_VAL;
        list -> data [index].next  = index + 1;
        list -> data [index].prev  = POISON_INDEX;   
    }
    list -> data [capacity].next = list -> free;
    list -> free = (int) (list -> capacity + 1);

    list -> capacity = capacity;

    ListVerify (list);
    return LIST_OK;
}


int List_verify (List_t *list)
{
    if (list == nullptr) return LIST_NULLPTR_ARG;

    if (list -> status != LIST_CONSTRUCTED) list -> err |= LIST_STATUS_ERROR;

    if (list -> info.     name == nullptr ||
        list -> info.file_name == nullptr ||
        list -> info.func_name == nullptr   ) list -> err |= LIST_INFO_CORRUPTED;

    if (list -> free < 0 || list -> free > list -> capacity) list -> err |= LIST_INCORRECT_INDEX;
    
    if (list -> data == nullptr)
    {
        list -> err |= LIST_NULLPTR_ARG;
        return list -> err;
    }

    #ifndef NDEBUG
    list -> err |= List_verify_data (list);
    #endif

    return list -> err;
}

int List_verify_data (List_t *list)
{
    if (list -> data [0].value != POISON_VAL)      return LIST_DATA_CORRUPTED;

    int index   = 0;
    int counter = 0;

    for (counter = 0; counter <= list -> capacity; counter++)
    {
        if (index < 0 || index > list -> capacity) return LIST_DATA_CORRUPTED;
        if (list -> data [index].value == POISON_VAL && index != 0) return LIST_DATA_CORRUPTED;

        int next = list -> data [index].next;

        if (next < 0 || next > list -> capacity)   return LIST_DATA_CORRUPTED;

        if (list -> data [next].prev != index)     return LIST_DATA_CORRUPTED;
        
        if (next == 0) break;

        index = next;
    }
    if (index != list -> data[0].prev) return LIST_INCORRECT_CAPACITY;

    index = list -> free;
    for (; counter < list -> capacity; counter++)
    {
        if (index == 0) return LIST_INCORRECT_CAPACITY;

        if (index < 0 || index > list -> capacity)      return LIST_DATA_CORRUPTED;
        if (list -> data [index].value != POISON_VAL  ) return LIST_DATA_CORRUPTED;
        if (list -> data [index].prev  != POISON_INDEX) return LIST_DATA_CORRUPTED;

        index = list -> data [index].next;
    }
    if (counter != list -> capacity || index != 0) return LIST_INCORRECT_CAPACITY;

    return LIST_OK;
}

void List_print_error (List_t *list, const char *func_name, const char *file_name, int line)
{
    FILE *log = fopen (LOGFILENAME, "a");
    FILE *stream = log == nullptr ? stderr : log;

    int err = list == nullptr ? LIST_NULLPTR_ARG : list -> err;
    
    fprintf (stream, "\nERROR (%d) in (%s) at (%s) at line (%d).\n", err, func_name, file_name, line);

    List_txt_dmup (list, stream, func_name, file_name, line);
}

void List_txt_dmup (List_t *list, FILE *stream, const char *func_name, const char *file_name, int line)
{
    if (stream == nullptr) stream = stdout;

    if (func_name == nullptr) func_name = "(NULL)";
    if (file_name == nullptr) file_name = "(NULL)";

    fprintf (stream, "\nList dump from (%s) at (%s) at line (%d):\n", func_name, file_name, line);

    if (list == nullptr)
    {
        fprintf (stream, "Unknown list (nullptr).\n");
        return;
    }

    if (list -> err & LIST_INFO_CORRUPTED) fprintf (stream, "Unknown list (info corrupted):\n");
    else fprintf (stream, "list [%p] \"%s\" at (%s) at (%s)(%d):\n", list, list -> info.     name, list -> info.func_name, 
                                                                           list -> info.file_name, list -> info.     line);
    fprintf (stream, "{\n");
    fprintf (stream, "\tstatus   = %d\n", list -> status);
    fprintf (stream, "\tcapacity = %d\n", list -> capacity);
    fprintf (stream, "\terror    = %d\n", list -> err);
    fprintf (stream, "\thappy    = %d\n", list -> happy);
    fprintf (stream, "\tshift    = %d\n", list -> shift);
    

    fprintf (stream, "\n\tdata[%p]:\n", list -> data);
    if (list -> data == nullptr) return;
    
    fprintf (stream, "\thead     = %d\n", list -> data[0].next);
    fprintf (stream, "\ttail     = %d\n", list -> data[0].prev);
    fprintf (stream, "\tfree     = %d\n", list -> free);

    fprintf (stream, "\n");

    fprintf (stream, "\tindex:");
    for (int index = 0; index <= list -> capacity; index++) fprintf (stream, "  %4d", index);
    
    fprintf (stream, "\n\tvalue:");
    for (int index = 0; index <= list -> capacity; index++)
    {
        if (list -> data[index].value == POISON_VAL) fprintf (stream, "   PSN");
        else fprintf (stream, "  %4d", list -> data[index].value);
    }

    fprintf (stream, "\n\tnext: ");
    for (int index = 0; index <= list -> capacity; index++) fprintf (stream, "  %4d", list -> data[index].next );
    
    fprintf (stream, "\n\tprev: ");
    for (int index = 0; index <= list -> capacity; index++) fprintf (stream, "  %4d", list -> data[index].prev );

    fprintf (stream, "\n}\n");
}


void *Recalloc (void *memptr, size_t num, size_t size, size_t old_num)
{
    memptr = realloc (memptr, num * size);
    if (memptr == NULL) return NULL;

    if (num > old_num) memset ((void *) ((char *) memptr + old_num * size), 0, (num - old_num) * size);

    return memptr;
}
#include "list.h"


#define ListErr(list, error)  {list -> err |= error;   \
                                ListPrintError (list); \
                                return list -> err; }


int List_ctor (List_t *list, int capacity, const char *name, const char *func_name, const char *file_name, int line)
{
    if (list == nullptr) return LIST_NULLPTR_ARG;

    if (list -> status != LIST_CREATED) ListErr (list, LIST_STATUS_ERROR);

    list -> err |= List_set_info (list, name, func_name, file_name, line);

    list -> err |= List_construct_data (list, capacity);

    list -> happy  = 1;
    list -> shift  = 0;
    list -> status = LIST_CONSTRUCTED;

    ListVerify (list);

    return LIST_OK;
}

int List_set_info (List_t *list, const char *name, const char *func_name, const char *file_name, int line)
{
    if (list == nullptr) return LIST_NULLPTR_ARG;

    list -> info.     name =      name;
    list -> info.func_name = func_name;
    list -> info.file_name = file_name;
    list -> info.     line =      line;

    return LIST_OK;
}

int List_construct_data (List_t *list, int capacity)
{
    if (list == nullptr) return LIST_NULLPTR_ARG;
    if (capacity < 0)    return LIST_INCORRECT_CAPACITY;

    list -> data = (ListElem_t *) (calloc (capacity + 1, sizeof (list -> data [0])));
    if (list -> data == nullptr) ListErr (list, LIST_ALLOC_ERROR);

    list -> capacity = capacity;

    List_fill_free (list -> data, 0, capacity);

    list -> data [capacity].next = 0;

    list -> free = list -> data [0].next;

    list -> data [0].prev = 0;
    list -> data [0].next = 0;

    return LIST_OK;
}

void List_fill_free (ListElem_t *data, int start, int end)
{
    for (int index = start; index <= end; index++)
    {
        data [index].next  = index + 1;
        data [index].prev  = POISON_INDEX;
        data [index].value = POISON_VAL;
    }
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


int ListInsert (List_t *list, int prev, val_t value)
{
    ListVerify (list);

    if (prev < 0 || prev > list -> capacity || list -> data [prev].prev == POISON_INDEX) ListErr (list, LIST_INCORRECT_INDEX);

    if (!list -> free)
    {
        if (ListResize (list, list -> capacity == 0 ? LIST_BASE_CAPACITY : list -> capacity * 2 + 1, 0)) return list -> err;
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
        if      (  prev == 0 && next == index + 1             ) list -> shift -= 1;
        else if (  prev == 0 && next == 0 && list -> shift > 0) list -> shift -= 1;
        else if (!(next == 0 && prev == index - 1)            ) list -> happy  = 0;
    }

    ListVerify (list);
    return LIST_OK;
}

int ListInsertHead (List_t *list, val_t value)
{
    ListVerify (list);
    return ListInsert (list, 0, value);
}

int ListInsertTail (List_t *list, val_t value)
{   
    ListVerify (list);
    return ListInsert (list, list -> data[0].prev, value);
}

int ListRemove (List_t *list, int index, val_t *value)
{
    ListVerify (list);

    if (index <= 0 || index > list -> capacity || list -> data [index].prev == POISON_INDEX) ListErr (list, LIST_INCORRECT_INDEX);

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

    if (position <= 0 || position > list -> capacity) ListErr (list, LIST_INCORRECT_INDEX);

    if (list -> happy)
    {
        if (position + list -> shift > list -> capacity) ListErr (list, LIST_INCORRECT_INDEX);
        *index = position + list -> shift;
    }
    else
    {
        int ind = list -> data [0].next;
        for (position--; position > 0; position--)
        {
            if (ind == 0) ListErr (list, LIST_INCORRECT_INDEX);
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
    return ListResize (list, list -> capacity);
}

int ListResize (List_t *list, int capacity, int do_linearize)
{
    ListVerify (list);

    if (capacity < 0)                                 ListErr (list, LIST_INCORRECT_CAPACITY);
    if (capacity < list -> capacity && !do_linearize) ListErr (list, LIST_INCORRECT_ARGS    );
    if (capacity == list -> capacity && (!do_linearize || list -> happy)) return LIST_OK;

    if (!do_linearize)
    {
        list -> data = (ListElem_t *) Recalloc (list -> data, capacity + 1, sizeof (list -> data [0]), list -> capacity + 1);
        if (list -> data == nullptr) ListErr (list, LIST_ALLOC_ERROR);

        List_fill_free (list -> data, list -> capacity + 1, capacity);

        list -> data [capacity].next = list -> free;
        list -> free = list -> capacity + 1;

        list -> capacity = capacity;

        list -> happy = 0;
        list -> shift = 0;

        ListVerify (list);
        return LIST_OK;
    }

    ListElem_t *new_data = (ListElem_t *) (calloc (capacity + 1, sizeof (list -> data [0])));
    if (new_data == nullptr) ListErr (list, LIST_ALLOC_ERROR);

    if (list -> happy)
    {
        int tail = list -> data [0].prev;
        int head = list -> data [0].next;

        if (tail > capacity) list -> happy = 0;
        else 
        {
            memcpy (new_data, list -> data, sizeof (list -> data[0]) * (1 + tail));

            list -> free = 0;

            if (head > 1)
            {
                List_fill_free (new_data, 1, head - 1);
                new_data [head - 1].next = list -> free;
                list -> free = 1;
            }
            if (tail < capacity)
            {
                List_fill_free (new_data, tail + 1, capacity);
                new_data [capacity].next = list -> free;
                list -> free = tail + 1;
            }
        }
    }

    if (!list -> happy)
    {
        list -> err |= List_linearize_data (list, new_data, capacity);
        if (list -> err)
        {
            free (new_data);
            ListPrintError (list);
            return list -> err;
        }
        list -> happy = 1;
        list -> shift = 0;
    }

    free (list -> data);
    list -> data = new_data;
    list -> capacity = capacity;

    ListVerify (list);
    return LIST_OK;
}

int List_linearize_data (List_t *list, ListElem_t *new_data, int capacity)
{
    int index_old = 0;
    int index_new = 0;

    for (;index_new <= list -> capacity; index_new++)
    {
        if (index_new > capacity) return LIST_INCORRECT_CAPACITY;

        new_data [index_new].next  = index_new + 1;
        new_data [index_new].prev  = index_new - 1;
        new_data [index_new].value = list -> data [index_old].value;

        index_old = list -> data [index_old].next;
        if (index_old == 0) break;
    }

    new_data [0]        .prev = index_new;
    new_data [index_new].next = 0;

    if (index_new == capacity)
    {
        list -> free = 0;
        return LIST_OK;
    }

    List_fill_free (new_data, index_new + 1, capacity);
    list -> free = index_new + 1;
    new_data [list -> capacity].next = 0;

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
    int err = list == nullptr ? LIST_NULLPTR_ARG : list -> err;
    
    printf ("\nERROR (%d) in (%s) at (%s) at line (%d).\n", err, func_name, file_name, line);

    List_dump (list, func_name, file_name, line);
}

void *Recalloc (void *memptr, size_t num, size_t size, size_t old_num)
{
    memptr = realloc (memptr, num * size);
    if (memptr == NULL) return NULL;

    if (num > old_num) memset ((void *) ((char *) memptr + old_num * size), 0, (num - old_num) * size);

    return memptr;
}
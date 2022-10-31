#include "list.h"

const char *const IMGNUMFILE = "imgnum.txt";
const char *const  GRAPHFILE =  "graph.txt";

const char *const NEXTCOLOR = "blue";
const char *const PREVCOLOR = "red";
const char *const FREECOLOR = "green";

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


void List_dump (List_t *list, const char *func_name, const char *file_name, int line)
{
    fprintf (LOG, "<pre>\n");
    List_txt_dmup (list, LOG, func_name, file_name, line);

    if (list != nullptr && list -> data != nullptr && list -> capacity >= 0)
    {
        int imgnum = 0;
        FILE *numfile = fopen (IMGNUMFILE, "r");
        if (numfile != nullptr)
        {
            fscanf (numfile, "%d", &imgnum);
            fclose (numfile);
            numfile = fopen (IMGNUMFILE, "w");
            fprintf (numfile, "%d", imgnum + 1);
            fclose (numfile);
        }
        Generate_img (list, imgnum);
        fprintf (LOG, "<img src=\"./images/dumpimg%d.png\", width=\"80%%\">", imgnum);
    }
}


void Generate_img (List_t *list, int imgnum)
{
    FILE *graph = fopen (GRAPHFILE, "w");
    if (graph == nullptr) return;

    fprintf (graph, "digraph {\n rankdir = LR;\n"
                    "node [shape = record, fontsize = 12, style = \"rounded, filled\", fillcolor = lightblue];\n"
                    "graph [splines = true];\n");
    
    fprintf (graph, "elem0 [color = red, style = \"rounded, filled\", fillcolor = white, label = \"index: 0|");
    if (list -> data [0].value == POISON_VAL)   fprintf (graph, "value = PSN");
    else                                        fprintf (graph, "value = %d", list -> data [0].value); 
    if (list -> data [0].prev  == POISON_INDEX) fprintf (graph, "|{tail: PSN");
    else                                        fprintf (graph, "|{tail: %d", list -> data [0].prev);
    if (list -> data [0].next  == POISON_INDEX) fprintf (graph, "|head: PSN}\"];\n");
    else                                        fprintf (graph, "|head: %d}\"];\n", list -> data [0].next);
    
    for (int index = 1; index <= list -> capacity; index++)
    {
        fprintf (graph, "elem%d [label = \"index: %d|", index, index);
        if (list -> data [index].value == POISON_VAL)   fprintf (graph, "value = PSN");
        else                                            fprintf (graph, "value = %d",   list -> data [index].value); 
        if (list -> data [index].prev  == POISON_INDEX) fprintf (graph, "|{prev: PSN");
        else                                            fprintf (graph, "|{prev: %d",   list -> data [index].prev);
        if (list -> data [index].next  == POISON_INDEX) fprintf (graph, "|next: PSN}\"");
        else                                            fprintf (graph, "|next: %d}\"", list -> data [index].next);

        if (list -> data [index].prev  == POISON_INDEX) fprintf (graph, "fillcolor = lightyellow];\n");
        else                                            fprintf (graph, "];\n");
    }

    fprintf (graph, "elem0");
    for (int index = 1; index <= list -> capacity; index++) fprintf (graph, " -> elem%d", index);
    fprintf (graph, " [style = invis, weight = 1000];\n");

    for (int index = 0; index <= list -> capacity; index++)
    {
        const char *next_color = NEXTCOLOR;

        if (list -> data [index].prev == POISON_INDEX) next_color = FREECOLOR;

        if (list -> data [index].next >= 0 && list -> data [index].next <= list -> capacity)
        {
            fprintf (graph, "elem%d -> elem%d [color = %s, weight = 1", index, list -> data [index].next, next_color);
            if (list -> data [index].next != index + 1) fprintf (graph, ", headport = n, tailport = n");
            fprintf (graph, "];\n");
        }
        if (list -> data [index].prev >= 0 && list -> data [index].prev <= list -> capacity)
        {
            fprintf (graph, "elem%d -> elem%d [color = %s, weight = 1", index, list -> data [index].prev, PREVCOLOR);
            if (list -> data [index].prev != index - 1) fprintf (graph, ", headport = s, tailport = s");
            fprintf (graph, "];\n");
        }
    }
    fprintf (graph, "free [label = \"free: %d\", style = filled, fillcolor = lightyellow];\n", list -> free);
    if (list -> free >= 0 && list -> free <= list -> capacity)
    {
        //fprintf (graph, "{rank = same; free; elem%d}", list -> free);
        fprintf (graph, "free -> elem%d [color = green, weight = 1, headport = n, tailport = s];", list -> free);
    }
    fprintf (graph, "}");

    fclose (graph);

    char cmd [64] = "";
    sprintf (cmd, "dot -T png -o ./images/dumpimg%d.png %s", imgnum, GRAPHFILE);
    system (cmd);
}
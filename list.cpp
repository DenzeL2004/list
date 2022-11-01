#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "list.h"

#include "src/log_info/log_errors.h"
#include "src/Generals_func/generals.h"


static int Init_list_data   (List *list);

static int List_resize      (List *list);

static int List_recalloc    (List *list, int ver_resize);

static void Init_node (Node *list_elem, elem_t val, int next, int prev);

static int Check_correct_ind (const List *list, const int ind);

static int List_data_not_free_verifier  (const List *list);

static int List_data_free_verifier      (const List *list);

static uint64_t Check_list (const List *list);


static int List_draw_logical_graph  (const List *list);

static int List_draw_physical_graph (const List *list);

static void Print_list_variables (const List *list, FILE *fpout);

static void Print_error_value (uint64_t err, FILE *fpout);

#define REPORT(...)                                         \
    {                                                       \
        List_dump (list, __VA_ARGS__);                      \
        Err_report ();                                      \
                                                            \
    }while (0)
                                    
//======================================================================================

int List_ctor (List *list, long capacity)
{
    assert (list != nullptr && "list is nullptr");

    if (capacity <= 0)
    {
        Log_report ("Incorrectly entered capacity values: %d\n", capacity);
        Err_report ();

        return LIST_CTOR_ERR;
    }

    list->tail_ptr = Dummy_element;
    list->head_ptr = Dummy_element;
    list->free_ptr = 1;

    list->is_linearized = 1;

    list->size_data      = 0;
    list->capacity       = capacity;
    list->cnt_free_nodes = 0;

    list->data = (Node*) calloc (capacity + 1, sizeof (Node));

    if (Check_nullptr (list->data))
    {
        Log_report ("Memory allocation error\n");
        Err_report ();

        return LIST_CTOR_ERR;
    }

    Init_node (&list->data[0], 
               Poison_val, Dummy_element, Dummy_element);


    if (Init_list_data (list))
    {
        Log_report ("Node initialization error\n");
        Err_report ();

        return LIST_CTOR_ERR;
    }

    return 0;
}

//======================================================================================

int List_dtor (List *list)
{
    assert (list != nullptr && "list ptr is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_dtor\n");
        return LIST_DTOR_ERR;
    }

    if (Check_nullptr (list->data))
        Log_report ("Data is nullptr in dtor\n");
    else    
        free (list->data);

    list->tail_ptr = Poison_ptr;
    list->head_ptr = Poison_ptr;
    list->free_ptr = Poison_ptr;

    list->size_data         = -1;
    list->capacity          = -1;
    list->cnt_free_nodes    = -1;
    
    list->is_linearized = -1;

    return 0;
}

//======================================================================================

static int Init_list_data (List *list)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: Init_list_data\n");
        return DATA_INIT_ERR;
    }

    int Last_used_node = MAX (list->tail_ptr, list->head_ptr);


    for (int ip = Last_used_node + 1; ip < list->capacity; ip++) 
        Init_node (list->data + ip, Poison_val, ip + 1, Identifier_free_node);

    Init_node (list->data + list->capacity, Poison_val, Identifier_free_node, Identifier_free_node);


    list->cnt_free_nodes = list->capacity - list->size_data;

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: Init_list_data\n");
        return DATA_INIT_ERR;
    }

    return 0;
}

//======================================================================================

static void Init_node (Node *list_elem, elem_t val, int next, int prev)
{
    assert (list_elem != nullptr && "list_elem ptr is nullptr");

    list_elem->val  = val;
    list_elem->next = next;
    list_elem->prev = prev; 

    return;
}

//======================================================================================

int List_insert_befor_ind (List *list, const int ind, const elem_t val) 
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_insert_befor_ind,"
                        " ind = %d, val = %d\n", ind, val);
        return LIST_INSERT_ERR;
    }
    
    int ver_resize = List_resize (list);
    if (List_recalloc (list, ver_resize))
    {
        Log_report ("Recalloc error\n");
        Err_report ();
        return LIST_INSERT_ERR;
    } 

    if (!Check_correct_ind (list, ind) && ind != Dummy_element)
    {
        Log_report ("Incorrect ind = %d\n", ind);
        return LIST_INSERT_ERR;
    }

    
    if (list->free_ptr == Dummy_element)
    {
        Log_report ("No free space in list\n");
        return LIST_INSERT_ERR;
    }

    if (list->data[ind].prev == Identifier_free_node)
    {
        Log_report ("There is nothing at this pointer: %d.\n" 
                    "You can only add an element before initialized elements\n", ind);
        return LIST_INSERT_ERR;
    }

        
    if (list->head_ptr != ind && list->tail_ptr != ind)
        list->is_linearized = 0;

    
    int  prev_ptr      = ind;
    int  cur_free_ptr  = list->free_ptr;

    list->free_ptr = list->data[cur_free_ptr].next;  
    
    Init_node (list->data + cur_free_ptr, 
               val, list->data[prev_ptr].next, prev_ptr);
    
    int next_ptr = list->data[cur_free_ptr].next;

    list->data[next_ptr].prev = cur_free_ptr;
    list->data[prev_ptr].next = cur_free_ptr;
    
    list->head_ptr = list->data[Dummy_element].next;
    list->tail_ptr = list->data[Dummy_element].prev;

    list->size_data++;
    list->cnt_free_nodes--;

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: List_insert_befor_ind,"
                        " ind = %d, val = %d\n", ind, val);
        return LIST_INSERT_ERR;
    }

    return cur_free_ptr;
}

//======================================================================================

int List_insert_front (List *list, const elem_t val) 
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_insert_front %d\n", val);
        return LIST_INSERT_ERR;
    }
    
    int ver_resize = List_resize (list);
    if (List_recalloc (list, ver_resize))
    {
        Log_report ("Recalloc error\n");
        Err_report ();
        return LIST_INSERT_ERR;
    } 
        
    if (list->size_data != 0)
        list->is_linearized = 0;

    
    int  prev_ptr      = Dummy_element;
    int  cur_free_ptr  = list->free_ptr;

    list->free_ptr = list->data[cur_free_ptr].next;  
    
    Init_node (list->data + cur_free_ptr, 
               val, list->data[prev_ptr].next, prev_ptr);
    
    int next_ptr = list->data[cur_free_ptr].next;

    list->data[next_ptr].prev = cur_free_ptr;
    list->data[prev_ptr].next = cur_free_ptr;
    
    list->head_ptr = list->data[Dummy_element].next;
    list->tail_ptr = list->data[Dummy_element].prev;

    list->size_data++;
    list->cnt_free_nodes--;

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: List_insert_front %d\n", val);
        return LIST_INSERT_ERR;
    }

    return cur_free_ptr;
}

//======================================================================================

int List_insert_back (List *list, const elem_t val) 
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_insert_back %d\n", val);
        return LIST_INSERT_ERR;
    }

    int ver_resize = List_resize (list);
    if (List_recalloc (list, ver_resize))
    {
        Log_report ("Recalloc error\n");
        Err_report ();
        return LIST_INSERT_ERR;
    } 
    
    int  prev_ptr      = list->tail_ptr;
    int  cur_free_ptr  = list->free_ptr;

    list->free_ptr = list->data[cur_free_ptr].next;  
    
    Init_node (list->data + cur_free_ptr, 
               val, list->data[prev_ptr].next, prev_ptr);
    
    int next_ptr = list->data[cur_free_ptr].next;

    list->data[next_ptr].prev = cur_free_ptr;
    list->data[prev_ptr].next = cur_free_ptr;
    
    list->head_ptr = list->data[Dummy_element].next;
    list->tail_ptr = list->data[Dummy_element].prev;

    list->size_data++;
    list->cnt_free_nodes--;

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: List_insert_back %d\n", val);
        return LIST_INSERT_ERR;
    }

    return cur_free_ptr;
}

//======================================================================================

int List_erase (List *list, const int ind) 
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_erase, ind = %d\n", ind);
        return LIST_ERASE_ERR;
    }   
    
    if (!Check_correct_ind (list, ind))
    {
        Log_report ("Incorrect ind = %d\n", ind);
        return LIST_ERASE_ERR;
    }


    if (list->data[ind].prev == Identifier_free_node)
    {
        Log_report ("There is nothing at this pointer: %d.\n" 
                    "You cannot free a previously freed node\n", ind);
        return LIST_ERASE_ERR;
    }


    if (list->head_ptr != ind && list->tail_ptr != ind)
        list->is_linearized = 0;
    

    int ver_resize = List_resize (list);
    if (List_recalloc (list, ver_resize))
    {
        Log_report ("Recalloc error\n");
        Err_report ();
        return LIST_INSERT_ERR;
    }


    int  cur_ptr   = ind;
    int  prev_ptr  = list->data[ind].prev;
    int  next_ptr  = list->data[ind].next;

    list->data[prev_ptr].next = next_ptr;
    list->data[next_ptr].prev = prev_ptr;

    Init_node (list->data + cur_ptr, 
               Poison_val, list->free_ptr, Identifier_free_node);

    list->free_ptr = cur_ptr;

    list->head_ptr = list->data[Dummy_element].next;
    list->tail_ptr = list->data[Dummy_element].prev;

    list->size_data--;
    list->cnt_free_nodes++;

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: List_erase exit, ind = %d\n", ind);
        return LIST_ERASE_ERR;
    }  

    return 0;
}

//======================================================================================

static int List_resize (List *list)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_resize\n");
        return LIST_RESIZE_ERR;
    }   

    if (list->capacity / 4  <= list->size_data   && 
        list->size_data + 1 < list->capacity / 2 && 
        list->is_linearized == 1)
    {
        list->capacity = list->capacity / 2 + 1;
        return 1;
    } 

    if (list->capacity == list->size_data + 1)
    {
        list->capacity = list->capacity * 2 + 1;
        return 1;
    }

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: List_resize\n");
        return LIST_RESIZE_ERR;
    }  

    return 0;
}

//======================================================================================

static int List_recalloc (List *list, int resize_status)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_recalloc\n");
        return LIST_RECALLOC_ERR;
    } 

    if (resize_status == 0) return 0;

    if (resize_status < 0)
    {
        Log_report ("The list is not subject to recalloc\n");
        Err_report ();
        return LIST_RECALLOC_ERR;
    }

    list->data = (Node*) realloc (list->data, (list->capacity + 1) * sizeof (Node));

    if (Check_nullptr (list->data))
    {
        Log_report ("List data is nullptr after use recalloc\n");
        Err_report ();
        return ERR_MEMORY_ALLOC;
    }

    
    list->cnt_free_nodes = 0;
    if (Init_list_data (list))
    {
        Log_report ("List data initialization error\n");
        Err_report ();
        return LIST_RECALLOC_ERR;
    }

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: List_recalloc\n");
        return LIST_RECALLOC_ERR;
    } 

    return 0;
}

//======================================================================================

int List_linearize (List *list)
{
    assert (list != nullptr && "list is nullptr\n");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_linearize\n");
        return LIST_LINEARIZE_ERR;
    } 

    if (list->is_linearized == 1) 
        return 0;

    Node* new_data = (Node*) calloc (list->capacity + 1, sizeof (Node));

    if (Check_nullptr (new_data))
    {
        Log_report ("Linearize error, new data memory allocation error\n");
        Err_report ();
        return ERR_MEMORY_ALLOC;
    }

    int logical_ind = list->head_ptr;

    Init_node (new_data, Poison_val, 1, list->size_data);

    int counter = 1;

    while (counter <= list->size_data - 1)
    {
        Init_node (new_data + counter, 
                   list->data[logical_ind].val, counter + 1, counter - 1);
        counter++;

        logical_ind = list->data[logical_ind].next;

        if (logical_ind != 0 && !Check_correct_ind (list, logical_ind))
        {
            Log_report ("Incorrect list traversal, logical_ind = %d\n", logical_ind);
            Err_report ();
            return LIST_LINEARIZE_ERR;
        }
    }

    Init_node (new_data + counter, 
               list->data[logical_ind].val, Dummy_element, counter - 1);


    free (list->data);

    list->cnt_free_nodes = 0;

    list->data = new_data;
    list->head_ptr = list->data[Dummy_element].next;
    list->tail_ptr = list->data[Dummy_element].prev;

    if (Init_list_data (list))
    {
        Log_report ("List data initialization error\n");
        Err_report ();
        return LIST_LINEARIZE_ERR;
    }

    list->is_linearized = 1;

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: List_linearize\n");
        return LIST_LINEARIZE_ERR;
    }

    return 0;;
}

//======================================================================================

int Get_ind_by_logical_order (const List *list, const int ind)
{
    assert (list != nullptr && "list is nullptr\n");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: Get_ind_by_logical_order, ind = %d\n", ind);
        return GET_LOGICAL_PTR_ERR;
    }   

    if (!Check_correct_ind (list, ind) && 
         list->data[ind].prev != Identifier_free_node)
    {
        Log_report ("Incorrect ind = %d\n", ind);
        return GET_LOGICAL_PTR_ERR;
    }

    if (ind > list->size_data)
    {
        Log_report ("Number of elements of the requested index.\nind = %d\n", ind);
        return GET_LOGICAL_PTR_ERR;
    }


    if (list->is_linearized)
    {
        return list->head_ptr + ind - 1;
    }

    else
    {   
        int logical_ind = list->head_ptr;
        int counter = 1;

        while (counter < ind)
        {
           logical_ind = list->data[logical_ind].next;
            counter++;            
        }
        
        return logical_ind;
    }

    //No list re-validation as list items don't change
    
} 

//======================================================================================

int List_get_val (const List *list, const int ind)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_get_val, ind = %d\n", ind);
        return GET_VAL_ERR; 
    }

    if (!Check_correct_ind (list, ind))
    {
        Log_report ("Incorrect ind = %d\n", ind);
        return GET_VAL_ERR;
    }

    //No list re-validation as list items don't change

    return list->data[ind].val;
}

//======================================================================================

int List_change_val (const List *list, const int ind, const elem_t val)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_change_val,"
                       " ind = %d, val = %d\n", ind, val);
        return Poison_val; 
    } 

    if (!Check_correct_ind (list, ind))
    {
        Log_report ("Incorrect ind = %d\n", ind);
        return Poison_val;
    }

    list->data[ind].val = val;

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: List_change_val,"
                       " ind = %d, val = %d\n", ind, val);
        return Poison_val; 
    }

    return 0;
}

//======================================================================================

static int Check_correct_ind (const List *list, const int ind)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: Check_correct_ind, ind = %d", ind);
        return CHECK_IND_ERR; 
    }

    if (ind < 0) return 0;

    if (ind == 0) return 0;

    if (ind > list->capacity) return 0;

    if (list->data[ind].val == Poison_val) return 0;
    

    //No list re-validation as list items don't change

    return 1;
}

//======================================================================================

static int List_data_not_free_verifier (const List *list)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_nullptr (list->data))
        return 1;

    int logical_ind = Dummy_element;
    int counter = 0;
        
    while (counter <= list->size_data)
    {
        if (logical_ind < 0) return 1;

        if (list->data[logical_ind].prev == Identifier_free_node) return 1;

        if (logical_ind != Dummy_element && list->data[logical_ind].val == Poison_val) return 1;
        
        logical_ind = list->data[logical_ind].next;
        counter++;            
    }

    if (logical_ind != Dummy_element) return 1;

    return 0;
}

//======================================================================================

static int List_data_free_verifier (const List *list)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_nullptr (list->data))
        return 1;

    int logical_ind = list->free_ptr;
    int counter = 1;
        
    while (counter <= list->cnt_free_nodes)
    {
        if (logical_ind < 0) return 1;
        
        if (list->data[logical_ind].prev != Identifier_free_node) return 1;

        if (list->data[logical_ind].val != Poison_val) return 1;

        
        logical_ind = list->data[logical_ind].next;
        counter++;            
    }

    return 0;
}

//======================================================================================

int List_dump_ (const List *list,
                const char* file_name, const char* func_name, int line, const char *format, ...)
{
    assert (list != nullptr && "list is nullptr\n");

    uint64_t err = Check_list (list);

    FILE *fp_logs = Get_log_file_ptr ();

    fprintf (fp_logs, "=================================================\n\n");

    va_list args = nullptr;

    va_start(args, format);
    fprintf (fp_logs, "<h2>");
    vfprintf(fp_logs, format, args);
    fprintf (fp_logs, "</h2>");
    va_end(args);

    fprintf (fp_logs, "REFERENCE:\n");

    if (err) 
        fprintf (fp_logs, "ERROR\nCaused an error in file %s, function %s, line %d\n\n", LOG_VAR);
    else
        fprintf (fp_logs, "OK\nlast call in file %s, function %s, line %d\n\n", LOG_VAR);
    
                     
    if (err)
    {
        fprintf (fp_logs, "ERR CODE: ");
        Bin_represent (fp_logs, err, sizeof (err));
        fprintf (fp_logs, "\n");
    }

    Print_list_variables (list, fp_logs);

    Print_error_value (err, fp_logs);

    #ifdef GRAPH_DUMP

        fprintf (fp_logs, "Logical graph\n");
        List_draw_logical_graph (list);
        
        fprintf (fp_logs, "\n\n");

        fprintf (fp_logs, "Physical graph\n");
        List_draw_physical_graph (list);
    
    #else

        for (int it = 0; it <= list->capacity; it++)
            fprintf (fp_logs, "%5d", it);
        fprintf (fp_logs, "\n");
        
        for (int it = 0; it <= list->capacity; it++)
            fprintf (fp_logs, "%5d", list->data[it].val);
        fprintf (fp_logs, "\n");
        
        for (int it = 0; it <= list->capacity; it++)
            fprintf (fp_logs, "%5d", list->data[it].next);
        fprintf (fp_logs, "\n");

        for (int it = 0; it <= list->capacity; it++)
            fprintf (fp_logs, "%5d", list->data[it].prev);
        fprintf (fp_logs, "\n");
    
    #endif
    
    fprintf (fp_logs, "\n");

    fprintf (fp_logs, "==========================================================\n\n");

    return 0;
}


//======================================================================================

static void Print_list_variables (const List *list, FILE *fpout)
{
    assert (list  != nullptr &&  "list is nullptr\n");
    assert (fpout != nullptr && "fpout is nullptr\n");

    fprintf (fpout, "<body>\n");
    fprintf (fpout, "<table border=\"1\">\n");
    
    fprintf (fpout, "<tr><td> data pointer </td> <td> %p </td></tr>", (char*) list->data);

    fprintf (fpout, "<tr><td> size data </td> <td>  %ld </td></tr>",  list->size_data);
    fprintf (fpout, "<tr><td> capacity </td> <td> %ld </td></tr>",    list->capacity);
    fprintf (fpout, "<tr><td>cnt free nodes</td><td> %ld </td></tr>", list->cnt_free_nodes);

    fprintf (fpout, "<tr><td> head pointer </td> <td>  %d </td></tr>",  list->head_ptr);
    fprintf (fpout, "<tr><td> tail pointer </td> <td>  %d </td></tr>",  list->tail_ptr);
    fprintf (fpout, "<tr><td> free pointer </td> <td>  %d </td></tr>",  list->free_ptr);

    fprintf (fpout, "<tr><td> is_linearized </td> <td>  %d </td></tr>",  list->is_linearized);

    fprintf (fpout, "</table>\n");
    fprintf (fpout, "</body>\n");
   
    return;
}

//======================================================================================

static void Print_error_value (uint64_t err, FILE *fpout)
{
    assert (fpout != nullptr && "fpout is nullptr\n");

    if (err & NEGATIVE_SIZE)
        fprintf (fpout, "Size_data is negative number\n");

    if (err & NEGATIVE_CAPAITY)
        fprintf (fpout, "Capacity is negative number\n");


    if (err & CAPACITY_LOWER_SIZE)
        fprintf (fpout, "Capacity is lower than size_data\n");

    if (err & DATA_IS_NULLPTR)
        fprintf (fpout, "Data pointer is nullptr\n");

    if (err & ILLIQUID_HEAD_PTR)
        fprintf (fpout, "Head pointer is incorrect\n");

    if (err & ILLIQUID_TAIL_PTR)
        fprintf (fpout, "Tail pointer is incorrect\n");

    if (err & ILLIQUID_FREE_PTR)
        fprintf (fpout, "Free pointer is incorrect\n");

    if (err & INCORRECT_LINEARIZED)
        fprintf (fpout, "Unknown linearize status\n"); 

    #ifdef LIST_DATA_CHECK

        if (err & DATA_NODE_INCORRECT)
            fprintf (fpout, "Сorrupted not-free nodes\n"); 


        if (err & DATA_FREE_NODE_INCORRECT)
            fprintf (fpout, "Сorrupted free nodes\n"); 
    
    #endif

    fprintf (fpout, "\n\n");

    return;
}

//======================================================================================

static int List_draw_logical_graph (const List *list)
{
    assert (list != nullptr && "list is nullptr\n");

    FILE *graph = Open_file_ptr ("graph_img/graph.txt", "w");
    if (Check_nullptr (graph))
    {
        Err_report ();
        return LIST_DRAW_GRAPH_ERR;
    }

    fprintf (graph, "digraph G{\n");
    fprintf (graph, "ranksep = 1.2;\n");
    fprintf (graph, "splines=ortho;\n");
    fprintf (graph, "{\n");

    
    fprintf (graph, "{rank=min;\n");

        if (list->head_ptr >= 0)
            fprintf (graph, "node_head [shape = circle, style=filled, color=coral, label=\"HEAD\"];\n");


        if (list->tail_ptr >= 0)
            fprintf (graph, "node_tail [shape = circle, style=filled, color=lightgreen, label=\"TAIL\"];\n");
        
        if (list->free_ptr >= 0)
            fprintf (graph, "node_free [shape = circle, style=filled, color=plum1, label=\"FREE\"];\n");

    fprintf (graph, "}\n");

    fprintf (graph, "node_head -> node%d\n", list->head_ptr);
    fprintf (graph, "node_tail -> node%d\n", list->tail_ptr);
    fprintf (graph, "node_free -> node%d\n", list->free_ptr);



    static int Cnt_graphs = 0;      //<-To display the current list view

    fprintf (graph, "{rank =  same;\n");

    for (int counter = 0; counter <= list->capacity; counter++) 
    {
        int next = list->data[counter].next;
        int prev = list->data[counter].prev;

        fprintf (graph, "node%d [style=filled, shape = record, label =  \"{NODE %d | VAL: %d| prev: %d | next: %d}}\",", 
                        counter, counter, list->data[counter].val, prev, next);

        if (prev != -1)
            fprintf (graph, " fillcolor=lightpink ];\n");
        else
            fprintf (graph, " fillcolor=lightskyblue ];\n");

        if (next != -1)
        {
            fprintf (graph, "node%d -> node%d[style=filled, fillcolor=yellow];\n", 
                             counter, next);
        }

        if (prev != -1)
        {
            fprintf (graph, "node%d -> node%d[style=filled, fillcolor=green];\n", 
                             counter, prev);
        }

        fprintf (graph, "\n");
    }


    fprintf(graph, "}\n}\n}\n");
    fclose(graph);

    char command_buffer[Max_command_buffer] = {0};
    sprintf(command_buffer, "dot -Tpng graph_img/graph.txt -o graph_img/picture_logical%d.png", Cnt_graphs);

    if (system(command_buffer))
    {
        Err_report ();
        return LIST_DRAW_GRAPH_ERR;
    }

    FILE *fp_logs = Get_log_file_ptr ();
    if (Check_nullptr (fp_logs))
    {
        Err_report ();
        return LIST_DRAW_GRAPH_ERR;
    }

    fprintf (fp_logs, "<img src= graph_img/picture_logical%d.png />\n", Cnt_graphs);
                                
    Cnt_graphs++;
    return 0;
}

//======================================================================================

static int List_draw_physical_graph (const List *list)
{
    assert (list != nullptr && "list is nullptr\n");

    FILE *graph = Open_file_ptr ("graph_img/graph.txt", "w");
    if (Check_nullptr (graph))
    {
        Err_report ();
        return LIST_DRAW_GRAPH_ERR;
    }

    fprintf (graph, "digraph G{\n");
    fprintf (graph, "ranksep = 1.2;\n");
    fprintf (graph, "splines=ortho;\n");
    fprintf (graph, "{\n");

    
    fprintf (graph, "{rank=min;\n");

        if (list->head_ptr >= 0)
            fprintf (graph, "node_head [shape = circle, style=filled, color=coral, label=\"HEAD\"];\n");


        if (list->tail_ptr >= 0)
            fprintf (graph, "node_tail [shape = circle, style=filled, color=lightgreen, label=\"TAIL\"];\n");
        
        if (list->free_ptr >= 0)
            fprintf (graph, "node_free [shape = circle, style=filled, color=plum1, label=\"FREE\"];\n");

    fprintf (graph, "}\n");

    fprintf (graph, "node_head -> node%d\n", list->head_ptr);
    fprintf (graph, "node_tail -> node%d\n", list->tail_ptr);
    fprintf (graph, "node_free -> node%d\n", list->free_ptr);



    static int Cnt_graphs = 0;      //<-To display the current list view

    fprintf (graph, "{rank =  same;\n");

    for (int counter = 0; counter <= list->capacity; counter++) 
    {
        int next = list->data[counter].next;
        int prev = list->data[counter].prev;

        fprintf (graph, "node%d [style=filled, shape = record, label =  \"{NODE %d | VAL: %d| prev: %d | next: %d}}\",", 
                        counter, counter, list->data[counter].val, prev, next);

        if (prev != -1)
            fprintf (graph, " fillcolor=lightpink ];\n");
        else
            fprintf (graph, " fillcolor=lightskyblue ];\n");

        if (next != -1)
        {
            fprintf (graph, "node%d -> node%d[style=filled, fillcolor=yellow];\n", 
                             counter, next);
        }

        if (prev != -1)
        {
            fprintf (graph, "node%d -> node%d[style=filled, fillcolor=green];\n", 
                             counter, prev);
        }

        fprintf (graph, "\n");
    
        if (counter != list->capacity)
        {
              fprintf (graph, "node%d -> node%d[style = invis, weight = 10000];\n", 
                             counter, counter + 1);
        }

        fprintf (graph, "\n"); 
    }


    fprintf(graph, "}\n}\n}\n");
    fclose(graph);

    char command_buffer[Max_command_buffer] = {0};
    sprintf(command_buffer, "dot -Tpng graph_img/graph.txt -o graph_img/picture_physical%d.png", Cnt_graphs);

    if (system(command_buffer))
    {
        Err_report ();
        return LIST_DRAW_GRAPH_ERR;
    }

    FILE *fp_logs = Get_log_file_ptr ();
    if (Check_nullptr (fp_logs))
    {
        Err_report ();
        return LIST_DRAW_GRAPH_ERR;
    }

    fprintf (fp_logs, "<img src= graph_img/picture_physical%d.png />\n", Cnt_graphs);
                                
    Cnt_graphs++;
    return 0;
}

//======================================================================================

static uint64_t Check_list (const List *list)
{
    assert (list != nullptr && "list is nullptr");

    uint64_t err = 0;

    if (list->size_data < 0) err |= NEGATIVE_SIZE;
    if (list->capacity  < 0) err |= NEGATIVE_CAPAITY;


    if (list->capacity  < list->size_data)    err |= CAPACITY_LOWER_SIZE;

    if (Check_nullptr (list->data))           err |= DATA_IS_NULLPTR;

    if (list->head_ptr == Poison_ptr                     ||
        list->head_ptr < 0                               ||  
        list->data[list->tail_ptr].next != Dummy_element   ) err |= ILLIQUID_HEAD_PTR;

    if (list->tail_ptr == Poison_ptr                     ||
        list->tail_ptr < 0                               ||  
        list->data[list->tail_ptr].next != Dummy_element   ) err |= ILLIQUID_TAIL_PTR;

    if (list->free_ptr <  0             || 
        list->free_ptr > list->capacity ||  
        list->free_ptr == Poison_ptr      ) err |= ILLIQUID_FREE_PTR;

    if ((list->is_linearized != 0) && (list->is_linearized != 1)) err |= INCORRECT_LINEARIZED; 

    #ifdef LIST_DATA_CHECK

        if (List_data_not_free_verifier (list))   err |= DATA_NODE_INCORRECT;

        if (List_data_free_verifier (list))       err |= DATA_FREE_NODE_INCORRECT;
    
    #endif

    return err;
}

//======================================================================================
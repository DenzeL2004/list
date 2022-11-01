#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "list.h"

#include "src/log_info/log_errors.h"
#include "src/Generals_func/generals.h"


static int Check_correct_ind (const List *list, const int ind);

static int List_nodes_verifier  (const List *list);

static uint64_t Check_list (const List *list);


static int List_draw_logical_graph (const List *list);

static void Print_list_variables (const List *list, FILE *fpout);


#define REPORT(...)                                         \
    {                                                       \
        List_dump (list, __VA_ARGS__);                      \
        Err_report ();                                      \
                                                            \
    }while (0)
                                    
//======================================================================================

int List_ctor (List *list)
{
    assert (list != nullptr && "list is nullptr");

    list->root = (Node*) calloc (1, sizeof (Node));

    if (Check_nullptr (list->root))
    {
        Log_report ("Memory allocation error, root = |%p|\n", 
                                             (char*) list->root);
        Err_report ();

        return LIST_CTOR_ERR;
    }

    list->root->next = list->root;
    list->root->prev = list->root;
    list->root->data = nullptr;

    list->head_ptr   = list->root;
    list->tail_ptr   = list->root;

    list->cnt_nodes = 0;

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

    Node *cur_ptr = list->head_ptr;
    int counter = 0;

    while (counter < list->cnt_nodes)
    {
        if (Check_nullptr (cur_ptr))
        {
            Log_report ("Current node is nullptr, next_node = |%p|", 
                                                   (char*) cur_ptr);
            Err_report ();
            return LIST_DTOR_ERR;
        }

        Node* next_ptr = cur_ptr->next;
        free (cur_ptr);

        cur_ptr = next_ptr;
        counter++;
    }

    if (cur_ptr != list->root)
    {
        Log_report ("Node memory free error\n");
        Err_report ();
        return LIST_DTOR_ERR;
    }

    if (Check_nullptr (list->root))
        Log_report ("root is nullptr in dtor\n");
    else    
        free (list->root);

    list->head_ptr = nullptr;
    list->tail_ptr = nullptr;

    list->cnt_nodes = -1;

    return 0;
}

//======================================================================================

int List_insert_befor_ptr (List *list, Node *node_ptr) 
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_insert_befor_ind,"
                        " node_ptr = |%p|\n", (char*) node_ptr);
        return LIST_INSERT_ERR;
    }
    
    if (Check_nullptr (node_ptr))
    {
        Log_report ("ptr is nullptr\n");
        return LIST_INSERT_ERR;
    }

    Node *new_node = (Node*) calloc (1, sizeof (Node));
    if (Check_nullptr (new_node))
    {
        Log_report ("Memory allocation error, new_node = |%p|\n", 
                                             (char*) new_node);
        Err_report ();

        return LIST_INSERT_ERR;
    }


    Node *next_ptr = node_ptr->next;

    node_ptr->next = new_node;
    next_ptr->prev = new_node;

    new_node->next = next_ptr;
    new_node->prev = node_ptr;
    new_node->data  = nullptr;

    list->head_ptr = list->root->next;
    list->tail_ptr = list->root->prev;

    list->cnt_nodes++;

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: List_insert_befor_ind,"
                        " node_ptr = |%p|\n", (char*) node_ptr);

        return LIST_INSERT_ERR;
    }

    return 0;
}

//======================================================================================

int List_erase (List *list, Node *node_ptr) 
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: List_erase,"
                       " ptr = |%d|\n", (char*) node_ptr);
        return LIST_ERASE_ERR;
    }   
    
    if (Check_nullptr (node_ptr) || node_ptr == list->root)
    {
        Log_report ("Incorrect node_ptr = |%p|\n", (char*) node_ptr);
        return LIST_INSERT_ERR;
    }

    Node *next_ptr = node_ptr->next;
    Node *prev_ptr = node_ptr->prev;

    next_ptr->prev = prev_ptr;
    prev_ptr->next = next_ptr;

    free (node_ptr);

    list->head_ptr = list->root->next;
    list->tail_ptr = list->root->prev;

    list->cnt_nodes--;

    if (Check_list (list))
    {
        REPORT ("EXIT\nFROM: List_erase,"
                       " node_ptr = |%d|\n", (char*) node_ptr);
        return LIST_ERASE_ERR;
    }    

    return 0;
}

//======================================================================================

Node* Get_pointer_by_logical_index (const List *list, const int ind)
{
    assert (list != nullptr && "list is nullptr\n");

    if (Check_list (list))
    {
        REPORT ("ENTRY\nFROM: Get_pointer_by_logical_index intput, ind = %d\n", ind);
        return nullptr;
    }   

    if (!Check_correct_ind (list, ind))
    {
        Log_report ("Incorrect ind = %d\n", ind);
        return nullptr;
    }


    Node *node_ptr = list->head_ptr;
    int counter = 1;

    while (counter < ind)
    {
        if (Check_nullptr (node_ptr))
        {
            Log_report ("node_ptr is nullptr");
            Err_report ();
            return nullptr;
        }

        node_ptr = node_ptr->next;
        counter++;            
    }
    
    //No list re-validation as list items don't change

    return node_ptr;
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

    if (ind > list->cnt_nodes) return 0;
    

    //No list re-validation as list items don't change

    return 1;
}

//======================================================================================

static int List_nodes_verifier (const List *list)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_nullptr (list->root))
        return 1;

    Node *cur_ptr = list->head_ptr;
    int counter = 0;

    while (counter < list->cnt_nodes)
    {
        if (Check_nullptr (cur_ptr)) return 1;

        Node* next_ptr = cur_ptr->next;
    
        cur_ptr = next_ptr;
        counter++;
    }

    if (cur_ptr != list->root) return 1;

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

    fprintf (fp_logs, "\n\n");

    #ifdef GRAPH_DUMP

        List_draw_logical_graph (list);
    
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
    
    fprintf (fpout, "<tr><td> pointer to root </td> <td> %p </td></tr>", (char*) list->root);

    fprintf (fpout, "<tr><td> cnt nodes </td> <td>  %ld </td></tr>",  list->cnt_nodes);

    fprintf (fpout, "<tr><td> head pointer </td> <td>  %p </td></tr>",  list->head_ptr);
    fprintf (fpout, "<tr><td> tail pointer </td> <td>  %p </td></tr>",  list->tail_ptr);

    fprintf (fpout, "</table>\n");
    fprintf (fpout, "</body>\n");
   
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
    fprintf (graph, "rankdir=LR;\n");
    fprintf (graph, "splines=spline;\n");
    fprintf (graph, "{\n");

    if (!Check_nullptr (list->head_ptr))
    {
        fprintf (graph, "node_head [shape = circle, style=filled, color=coral, label=\"HEAD\"];\n");
        fprintf (graph, "node_head -> node%p\n", (char*) list->head_ptr);
    }

    if (!Check_nullptr (list->tail_ptr))
    {
        fprintf (graph, "node_tail [shape = circle, style=filled, color=lightgreen, label=\"TAIL\"];\n");
        fprintf (graph, "node_tail -> node%p\n", (char*) list->tail_ptr);
    }

    Node *cur_node = list->root;

    for (int counter = 0; counter <= list->cnt_nodes; counter++) 
    {
        char* ch_next_node_ptr = (char*) cur_node->next;
        char* ch_prev_node_ptr = (char*) cur_node->prev;

        char *ch_ptr = (char*) cur_node;

        fprintf (graph, "node%p [style=filled, shape = record, label =  \"NODE POINTER %p | prev: %p | next: %p}\",", 
                        ch_ptr, ch_ptr, ch_prev_node_ptr, ch_next_node_ptr);


        fprintf (graph, " fillcolor=lightskyblue ];\n");

       
        fprintf (graph, "node%p -> node%p[style=filled, fillcolor=yellow];\n", 
                            ch_ptr, ch_next_node_ptr);
        

    
        fprintf (graph, "node%p -> node%p[style=filled, fillcolor=green];\n", 
                            ch_ptr, ch_prev_node_ptr);
    

        
        fprintf (graph, "\n");

        cur_node = cur_node->next;
    }


    fprintf(graph, "}\n}\n");
    fclose(graph);

    
    static int Cnt_graphs = 0;      //<-To display the current list view

    char command_buffer[Max_command_buffer] = {0};
    sprintf(command_buffer, "dot -Tpng graph_img/graph.txt -o graph_img/picture%d.png", Cnt_graphs);

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

    fprintf (fp_logs, "<img src= graph_img/picture%d.png />\n", Cnt_graphs);
                                
    Cnt_graphs++;
    return 0;
}

//======================================================================================

static uint64_t Check_list (const List *list)
{
    assert (list != nullptr && "list is nullptr");

    uint64_t err = 0;

    if (list->cnt_nodes < 0) err |= NEGATIVE_CNT;


    if (Check_nullptr (list->root))           err |= ROOT_IS_NULLPTR;

    if (Check_nullptr (list->head_ptr)     ||                           
        list->head_ptr->prev != list->root   ) err |= ILLIQUID_HEAD_PTR;

    if (Check_nullptr (list->tail_ptr)     ||                           
        list->tail_ptr->next != list->root   ) err |= ILLIQUID_TAIL_PTR;

    #ifdef LIST_DATA_CHECK

        if (List_nodes_verifier (list))   err |= DATA_NODE_INCORRECT;
    
    #endif

    return err;
}

//======================================================================================
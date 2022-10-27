#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "list.h"

#include "src/log_info/log_errors.h"
#include "src/Generals_func/generals.h"


static int Init_list_data   (List *list);

static int List_resize      (List *list);

static int List_recalloc    (List *list, int ver_resize);

static int Init_node (Node *list_elem, elem_t val, int next, int prev);

static int Check_correct_ind (const List *list, const int ind);

static int List_data_not_free_verifier  (const List *list);

static int List_data_free_verifier      (const List *list);

static uint64_t Check_list (const List *list);


#define SHUTDOWN_FUNC(...)              \
    {                                   \
        List_dump (list);               \
        Err_report ();                  \
                                        \
        __VA_ARGS__ ;                   \
    }while (0)
                                    
//======================================================================================

int List_ctor (List *list, long capacity)
{
    assert (list != nullptr && "list is nullptr");

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
        SHUTDOWN_FUNC (return LIST_DTOR_ERR);

    if (Check_nullptr (list->data))
        Log_report ("Data is nullptr in dtor");
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
        SHUTDOWN_FUNC (return DATA_INIT_ERR);


    int Last_used_node = MAX (list->tail_ptr, list->head_ptr);


    for (int ip = Last_used_node + 1; ip < list->capacity; ip++) 
        Init_node (list->data + ip, Poison_val, ip + 1, Identifier_free_node);

    Init_node (list->data + list->capacity, Poison_val, Identifier_free_node, Identifier_free_node);


    list->cnt_free_nodes = list->capacity - list->size_data;

    if (Check_list (list))
        SHUTDOWN_FUNC (return DATA_INIT_ERR);

    return 0;
}

//======================================================================================

static int Init_node (Node *list_elem, elem_t val, int next, int prev)
{
    assert (list_elem != nullptr && "list_elem ptr is nullptr");

    list_elem->val  = val;
    list_elem->next = next;
    list_elem->prev = prev; 

    return 0;
}

//======================================================================================

int List_insert (List *list, const int ind, const elem_t val) 
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
        SHUTDOWN_FUNC (return LIST_INSERT_ERR);
    
    int ver_resize = List_resize (list);
    if (List_recalloc (list, ver_resize))
    {
        Log_report ("Recalloc error\n");
        Err_report ();
        return LIST_INSERT_ERR;
    } 

    if (!Check_correct_ind (list, ind) && ind != Dummy_element)
    {
        Log_report ("Uncorrect ind = %d\n", ind);
        return LIST_INSERT_ERR;
    }

    
    if (list->free_ptr == Dummy_element)
    {
        Log_report ("No free space in list\n");
        return LIST_INSERT_ERR;
    }

    if (list->data[ind].prev == Identifier_free_node)
    {
        Log_report ("There is nothing at thispointer: %d.\n" 
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
        SHUTDOWN_FUNC (return LIST_INSERT_ERR);

    return cur_free_ptr;
}

//======================================================================================

int List_erase (List *list, const int ind) 
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
        SHUTDOWN_FUNC (return LIST_ERASE_ERR);    
    
    if (!Check_correct_ind (list, ind))
    {
        Log_report ("Uncorrect ind = %d\n", ind);
        return LIST_ERASE_ERR;
    }


    if (list->data[ind].prev == Identifier_free_node)
    {
        Log_report ("There is nothing at thispointer: %d.\n" 
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
        SHUTDOWN_FUNC (return LIST_ERASE_ERR);   

    return 0;
}

//======================================================================================

static int List_resize (List *list)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
        SHUTDOWN_FUNC (return LIST_RESIZE_ERR);   

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
        SHUTDOWN_FUNC (return LIST_RESIZE_ERR); 

    return 0;
}

//======================================================================================

static int List_recalloc (List *list, int resize_status)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
        SHUTDOWN_FUNC (return LIST_RECALLOC_ERR);  

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

    if (Init_list_data (list))
    {
        Log_report ("List data initialization error\n");
        Err_report ();
        return LIST_RECALLOC_ERR;
    }

    if (Check_list (list))
        SHUTDOWN_FUNC (return LIST_RECALLOC_ERR); 

    return 0;
}

//======================================================================================

int List_linearize (List *list)
{
    assert (list != nullptr && "list is nullptr\n");

    if (Check_list (list))
        SHUTDOWN_FUNC (return LIST_LINEARIZE_ERR);

    if (list->is_linearized == 1) 
        return 0;

    Node* new_data = (Node*) calloc (list->capacity + 1, sizeof (Node));

    if (Check_nullptr (new_data))
    {
        Log_report ("Linireze error, new data memory allocation error\n");
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
    if (Init_list_data (list))
    {
        Log_report ("List data initialization error\n");
        Err_report ();
        return LIST_LINEARIZE_ERR;
    }

    list->head_ptr = list->data[Dummy_element].next;
    list->tail_ptr = list->data[Dummy_element].prev;

    list->is_linearized = 1;


    if (Check_list (list))
        SHUTDOWN_FUNC (return LIST_LINEARIZE_ERR);

    return 0;;
}

//======================================================================================

int Get_pointer_by_logical_index (const List *list, const int ind)
{
    assert (list != nullptr && "list is nullptr\n");

    if (Check_list (list))
        SHUTDOWN_FUNC (return GET_LOGICAL_PTR_ERR);   

    if (!Check_correct_ind (list, ind) && 
         list->data[ind].prev != Identifier_free_node)
    {
        Log_report ("Uncorrect ind = %d\n", ind);
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
        SHUTDOWN_FUNC (return GET_VAL_ERR); 

    if (!Check_correct_ind (list, ind))
    {
        Log_report ("Uncorrect ind = %d\n", ind);
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
        SHUTDOWN_FUNC (return Poison_val); 

    if (!Check_correct_ind (list, ind))
    {
        Log_report ("Uncorrect ind = %d\n", ind);
        return Poison_val;
    }

    list->data[ind].val = val;

    if (Check_list (list))
        SHUTDOWN_FUNC (return Poison_val);

    return 0;
}

//======================================================================================

static int Check_correct_ind (const List *list, const int ind)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
        SHUTDOWN_FUNC (return CHECK_IND_ERR); 

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
                const char* file_name, const char* func_name, int line)
{
    assert (list != nullptr && "list is nullptr\n");

    uint64_t err = Check_list (list);

    FILE *fp_logs = Get_log_file_ptr ();

    fprintf (fp_logs, "=================================================\n\n");

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

    fprintf (fp_logs, "List pointer to data is |%p|\n\n", (char*) list->data);

    fprintf (fp_logs, "list size_data = %ld\n",      list->size_data);
    fprintf (fp_logs, "list capacity  = %ld\n",      list->capacity);
    fprintf (fp_logs, "list cnt_free_nodes = %ld\n", list->cnt_free_nodes);

    fprintf (fp_logs, "\n");
    
    fprintf (fp_logs, "list head_ptr = %d\n", list->head_ptr);
    fprintf (fp_logs, "list tail_ptr = %d\n", list->tail_ptr);
    fprintf (fp_logs, "list free_ptr = %d\n", list->free_ptr);

    fprintf (fp_logs, "\n");

    fprintf (fp_logs, "list is_linearized = %d\n", list->is_linearized);

    fprintf (fp_logs, "\n");


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

    fprintf (fp_logs, "\n");

    fprintf (fp_logs, "==========================================================\n\n");

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

    if (list->free_ptr <  0             || 
        list->free_ptr > list->capacity ||  
        list->free_ptr == Poison_ptr      )   err |= ILLIQUID_FREE_PTR;

    if ((list->is_linearized != 0) && (list->is_linearized != 1)) err |= UNCORRECT_LINEARIZED; 

    #ifdef LIST_DATA_NODE_VER

        if (List_data_not_free_verifier (list))   err |= DATA_NODE_UNCORRECT;
    
    #endif

    #ifdef LIST_DATA_FREE_NODE_VER

        if (List_data_free_verifier (list))       err |= DATA_FREE_NODE_UNCORRECT;
    
    #endif

    return err;
}

//======================================================================================
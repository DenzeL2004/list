#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "list.h"

#include "src/log_info/log_errors.h"
#include "src/Generals_func/generals.h"


static int Init_list_data (List *list);

static int Clear_list_data (List *list);

static int Init_node (Node *list_elem, elem_t val, int next, int prev);

static int Check_correct_ind (const List *list, const int ind);

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

    list->size_data = 0;
    list->capacity  = capacity;

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

    if (Clear_list_data (list))
    {
        Log_report ("Cleaning date error\n");
        Err_report ();

        return LIST_DTOR_ERR;
    }

    if (Check_nullptr (list->data))
        Log_report ("Data is nullptr in dtor");
    else    
        free (list->data);

    list->tail_ptr = Poison_ptr;
    list->head_ptr = Poison_ptr;
    list->free_ptr = Poison_ptr;

    list->size_data    = -1;
    list->capacity     = -1;
    
    list->is_linearized = -1;

    return 0;
}

//======================================================================================

static int Init_list_data (List *list)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
        SHUTDOWN_FUNC (return DATA_INIT_ERR);

    Node *list_data = list->data;

    for (int ip = list_data->prev + 1; ip < list->capacity; ip++)
        Init_node (list_data + ip, Poison_val, ip + 1, Identifier_free_cell);
        
    Init_node (list_data + list->capacity, Poison_val, Identifier_free_cell, Identifier_free_cell);

    if (Check_list (list))
        SHUTDOWN_FUNC (return DATA_INIT_ERR);

    return 0;
}

//======================================================================================

static int Clear_list_data (List *list)
{
    assert (list != nullptr && "list is nullptr");

    if (Check_list (list))
        SHUTDOWN_FUNC (return DATA_CLEAR_ERR);

    Node *list_data = list->data;

    for (int ip = 0; ip <= list->capacity; ip++)
        Init_node (&list_data[ip], Poison_val, Identifier_free_cell, Identifier_free_cell);

    if (Check_list (list))
        SHUTDOWN_FUNC (return DATA_CLEAR_ERR);

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
       
    /*if (Check_resize ())
    {

    } */

    if (!Check_correct_ind (list, ind) && ind != Dummy_element)
    {
        Log_report ("Uncorrevt ind\n");
        return LIST_INSERT_ERR;
    }
    
    if (list->head_ptr != ind && list->tail_ptr != ind)
        list->is_linearized = 0;
    
    if (list->free_ptr == Dummy_element)
    {
        Log_report ("No free space in list\n");
        return LIST_INSERT_ERR;
    }

    if (list->data[ind].prev == Identifier_free_cell)
    {
        Log_report ("There is nothing at thispointer: %d.\n" 
                    "You can only add an element before initialized elements\n", ind);
        return LIST_INSERT_ERR;
    }

    if (list->head_ptr < ind && list->tail_ptr > ind)
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
    //empty if (list -> nextFree == list -> head) return ListIsEmpty; 
    //  TODO: recalloc
    
    if (!Check_correct_ind (list, ind))
    {
        Log_report ("Uncorrevt ind.\nind = %d\n", ind);
        return LIST_ERASE_ERR;
    }


    if (list->head_ptr != ind && list->tail_ptr != ind)
        list->is_linearized = 0;


    if (list->data[ind].prev == Identifier_free_cell)
    {
        Log_report ("There is nothing at thispointer: %d.\n" 
                    "You cannot free a previously freed cell\n", ind);
        return LIST_ERASE_ERR;
    }
    
    int  cur_ptr   = ind;
    int  prev_ptr  = list->data[ind].prev;
    int  next_ptr  = list->data[ind].next;

    list->data[prev_ptr].next = next_ptr;
    list->data[next_ptr].prev = prev_ptr;

    Init_node (list->data + cur_ptr, 
               Poison_val, list->free_ptr, Identifier_free_cell);

    list->free_ptr = cur_ptr;

    list->head_ptr = list->data[Dummy_element].next;
    list->tail_ptr = list->data[Dummy_element].prev;

    list->size_data--;

    if (Check_list (list))
        SHUTDOWN_FUNC (return LIST_ERASE_ERR);   

    return 0;
}

//======================================================================================

int Get_pointer_by_logical_index (const List *list, const int ind)
{
    assert (list != nullptr && "list is nullptr\n");

    if (Check_list (list))
        SHUTDOWN_FUNC (return GET_LOGICAL_PTR_ERR);   

    if (!Check_correct_ind (list, ind) && 
         list->data[ind].prev != Identifier_free_cell)
    {
        Log_report ("Uncorrevt ind.\nind = %d\n", ind);
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
        Log_report ("Uncorrevt ind.\nind = %d\n", ind);
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
        Log_report ("Uncorrevt ind.\nind = %d\n", ind);
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

    if (ind < 0)
    {
        Log_report ("You've reached a negative pointer.\nind = %d\n", ind);
        return 0;
    }

    if (ind == 0)
    {
        Log_report ("Attempt to remove dummy element.\nind = %d\n", ind);
        return 0;
    }

    if (ind > list->capacity)
    {
        Log_report ("Accessing unallocated memory.\nind = %d\n", ind);
        return 0;
    }

    if (list->data[ind].val == Poison_val)
    {
        Log_report ("Accessing an uninitialized node.\nind = %d\n", ind);
        return 0;
    }

    //No list re-validation as list items don't change

    return 1;
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

    fprintf (fp_logs, "List size_data = %ld\n", list->size_data);
    fprintf (fp_logs, "List capacity  = %ld\n", list->capacity);

    fprintf (fp_logs, "\n");
    
    fprintf (fp_logs, "list free_ptr = %d\n", list->free_ptr);

    fprintf (fp_logs, "\n");

    fprintf (fp_logs, "list is_linearized = %d\n", list->is_linearized);

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

    return err;
}

//======================================================================================
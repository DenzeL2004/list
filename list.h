#ifndef _LIST_H_
#define _LIST_H_

#include "config_list.h"
#include "src/log_info/log_def.h"

const int Identifier_free_node = -1;

const int Dummy_element = 0;

const int Max_comand_buffer = 100;

struct Node
{
    elem_t val = 0;
    int next = 0;
    int prev = 0;
};

struct List
{
    long capacity       = 0;
    long size_data      = 0; 
    long cnt_free_nodes = 0;

    Node *data = nullptr;

    int head_ptr  = 0;
    int tail_ptr  = 0;
    int free_ptr  = 0;

    int is_linearized = 0; 
};


enum List_func_err
{
    LIST_CTOR_ERR           = -1,
    LIST_DTOR_ERR           = -2,

    LIST_INVALID_ERR        = -3,

    INDEFINITI_PTR_ERR      = -4,

    FREE_PTR_IS_DUMMY_ERR   = -5,

    PTR_OUT_OF_MEMOY_ERR    = -6,
    
    LIST_INSERT_ERR         = -7,
    LIST_ERASE_ERR          = -8,

    DATA_INIT_ERR           = -9,
    DATA_CLEAR_ERR          = -10,

    GET_LOGICAL_PTR_ERR     = -11,
    GET_VAL_ERR             = -12,

    CHECK_IND_ERR           = -13,

    LIST_RESIZE_ERR         = -14,    
    LIST_RECALLOC_ERR       = -15,

    LIST_LINEARIZE_ERR      = -16,
    
    LIST_DRAW_GRAPH_ERR     = -17,
};

enum List_err
{
    NEGATIVE_SIZE               = (1 << 0),
    NEGATIVE_CAPAITY            = (1 << 1),
    CAPACITY_LOWER_SIZE         = (1 << 2),

    ILLIQUID_FREE_PTR           = (1 << 3),

    DATA_IS_NULLPTR             = (1 << 4),

    UNCORRECT_LINEARIZED        = (1 << 5),

    DATA_NODE_UNCORRECT         = (1 << 6),
    DATA_FREE_NODE_UNCORRECT    = (1 << 7),
    
};



int List_ctor (List *list, const long capacity);

int List_dtor (List *list);


/** 
 * @brief Adds a node to the list
 * @version 1.0.0
 * @param [in] *list Structure List pointer
 * @param [in] ind The index of the memory location before which we want to add a new node. (The node at the given index must be initialized)
 * @param [in] val The value of the added node
 * @return If a vertex has been added, it returns the Physical Pointer where the element is located, otherwise a negative number
*/
int List_insert_befor_ind (List *list, const int ind, const elem_t val);

int List_insert_front     (List *list, const elem_t val);

int List_insert_back      (List *list, const elem_t val);


/** 
 * @brief Removes a node by its index
 * @version 1.0.0
 * @param [in] *list Structure List pointer
 * @param [in] ind The pointer by which we will delete the node. (The node at the given index must be initialized)
 * @return Returns zero if the node is deleted, otherwise returns a non-zero number
*/
int List_erase (List *list, const int ind);



int Get_pointer_by_logical_index (const List *list, const int ind);


/**
 * @brief Get value by physical index
 * @param [in] *list Structure List pointer
 * @param [in] ind The physical index by which we will get the node. (The node at the given index must be initialized)
 * @return Returns a poison value if an element referencing error has occurred, otherwise the return value is assumed to be the actual value of the node
*/
int List_get_val    (const List *list, const int ind);

/** 
 * @brief Change value by physical index
 * @version 1.0.0
 * @param [in] *list Structure List pointer
 * @param [in] ind The pointer by which we will chage the node. (The node at the given index must be initialized)
 * @return Returns zero if the node is change, otherwise returns a non-zero number
*/
int List_change_val (const List *list, const int ind, const int val);

int List_linearize (List *list);

#define List_dump(list)                       \
        List_dump_ (list, LOG_ARGS)

int List_dump_ (const List *list, LOG_PARAMETS);

#endif  //#endif _LIST_H_
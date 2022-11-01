#ifndef _LIST_H_
#define _LIST_H_

#include "config_list.h"
#include "src/log_info/log_def.h"

const int Max_command_buffer = 100;

struct Node
{
    elem_t *data = nullptr;
    Node   *next = nullptr;
    Node   *prev = nullptr;
};

struct List
{
    long cnt_nodes      = 0; 

    Node *root = nullptr;

    Node *head_ptr  = nullptr;
    Node *tail_ptr  = nullptr;
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
    NEGATIVE_CNT                = (1 << 0),

    ROOT_IS_NULLPTR             = (1 << 1),

    DATA_NODE_INCORRECT         = (1 << 2),

    ILLIQUID_HEAD_PTR           = (1 << 3),
    ILLIQUID_TAIL_PTR           = (1 << 4),
    
};


int List_ctor (List *list);

int List_dtor (List *list);


/** 
 * @brief Adds a node to the list
 * @version 1.0.0
 * @param [in] list Structure List pointer
 * @param [in] node_ptr The index of the memory location before which we want to add a new node. (The node at the given pointer must be initialized)
 * @return Returns zero if the node is added, otherwise returns a non-zero number
*/
int List_insert_befor_ptr (List *list, Node *node_ptr);

/** 
 * @brief Removes a node by its pointer
 * @version 1.0.0
 * @param [in] list Structure List pointer
 * @param [in] node_ptr The pointer by which we will delete the node. (The node at the given pointer must be initialized)
 * @return Returns zero if the node is deleted, otherwise returns a non-zero number
*/
int List_erase (List *list, Node *node_ptr);


Node* Get_pointer_by_logical_index (const List *list, const int ind);

#define List_dump(list, ...)                       \
        List_dump_ (list, LOG_ARGS, __VA_ARGS__)

int List_dump_ (const List *list, LOG_PARAMETS, const char *format, ...);

#endif  //#endif _LIST_H_
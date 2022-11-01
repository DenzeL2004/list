#ifndef _LIST_CONFIG_H_
#define _LIST_CONFIG_H_

#include <stdint.h>

typedef int elem_t;


const elem_t Poison_val = 404;  //<- In free nodes is written

const int Poison_ptr  = -126;   //<- Written to a pointer when the list is cleared

#define USE_LOG                 //<- connect when we use logs


#define LIST_DATA_CHECK      //<- Checking non-free list nodes for correct transitions and values

#define GRAPH_DUMP

#define ELEM_T_SPEC "d"            //<- specifier character to print elem

#endif  //endif _LIST_CONFIG_H_
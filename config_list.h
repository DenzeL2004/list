#ifndef _LIST_CONFIG_H_
#define _LIST_CONFIG_H_

#include <stdint.h>

typedef int elem_t;


const elem_t Poison_val = 404;

const int Poison_ptr  = -126;

#define USE_LOG         //<- connect when we use logs

#define DEBUG           //<- Use of protection

#ifdef DEBUG


#endif


#define USE_TYPE "d"               //<- specifier character to print elem

#endif  //endif _LIST_CONFIG_H_
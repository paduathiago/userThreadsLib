#include "dccthread.h"
#include "dlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

typedef struct dccthread{
    char *name;
	ucontext_t context;
}dccthread_t;

void dccthread_init(void (*func)(int), int param)
{
    

}

dccthread_t * dccthread_create(const char *name, void (*func)(int), int param)
{

}
#include "dccthread.h"
#include "dlist.h"

#include<malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

struct dlist *ready_list;

typedef struct dccthread{
    char *name;
	ucontext_t context;
}dccthread_t;

void dccthread_init(void (*func)(int), int param)
{
    ready_list = dlist_create();


}

dccthread_t * dccthread_create(const char *name, void (*func)(int), int param)
{
    dccthread_t *new_thread = (dccthread_t *) malloc(sizeof(dccthread_t));
    new_thread->name = (char *) malloc(sizeof(char) * strlen(name));
    strcpy(new_thread->name, name);
    getcontext(&new_thread->context);
    

    dlist_push_right(ready_list, new_thread);
}

void dccthread_yield(void)
{

}

dccthread_t * dccthread_self(void)
{
    
}

const char * dccthread_name(dccthread_t *tid)
{
    return tid->name;
}
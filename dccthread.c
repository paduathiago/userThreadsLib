#include "dccthread.h"
#include "dlist.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#define SIGSTKSZ 8192

struct dlist *ready_threads_list;
struct dlist* waiting_threads_list;

dccthread_t *manager_thread;
dccthread_t *current_thread;

typedef struct dccthread
{
    char *name;
	ucontext_t context;
    dccthread_t *waiting_for;
}dccthread_t;

void new_thread_stack(dccthread_t *thread)
{
    thread->context.uc_stack.ss_sp = malloc(SIGSTKSZ);
    thread->context.uc_stack.ss_size = SIGSTKSZ;
    thread->context.uc_stack.ss_flags = 0;
}

/*void next_up()
{
    // what if there are no more threads to run?
    if (dlist_empty(ready_threads_list))
        return;
    
    dccthread_t *next_thread = dlist_pop_left(ready_threads_list);
    
    // current thread must be added to the end of the ready list
    current_thread = next_thread;
    setcontext(&(current_thread->context));
}*/

void dccthread_init(void (*func)(int), int param)
{
    ready_threads_list = dlist_create();
    
    dccthread_create("main", func, param); // create main thread
    
    manager_thread = (dccthread_t *) malloc(sizeof(dccthread_t));
    manager_thread -> name = (char *) malloc(sizeof(char) * strlen("manager"));
    manager_thread -> name = "manager";
    getcontext(&(manager_thread->context));
    manager_thread->context.uc_link = NULL;
    new_thread_stack(manager_thread);
    
    while (!dlist_empty(ready_threads_list))
    {
        current_thread = (dccthread_t *) ready_threads_list->head->data;
        swapcontext(&(manager_thread->context), &(current_thread->context));
        dlist_pop_left(ready_threads_list);
    }

    exit(EXIT_SUCCESS);
}

dccthread_t * dccthread_create(const char *name, void (*func)(int), int param)
{
    dccthread_t *new_thread = (dccthread_t *) malloc(sizeof(dccthread_t));
    new_thread->name = (char *) malloc(sizeof(char) * strlen(name));
    strcpy(new_thread->name, name);
    
    getcontext(&(new_thread->context));
    new_thread->context.uc_link = &(manager_thread->context);
    new_thread_stack(new_thread);
    dlist_push_right(ready_threads_list, new_thread);
    makecontext(&(new_thread->context), (void (*)())func, 1, param);
    
    return new_thread;
}

void dccthread_yield(void)
{
    dlist_push_right(ready_threads_list, current_thread);	
	swapcontext(&(current_thread->context), &(manager_thread->context));
}

void dccthread_exit(void)
{
    free(current_thread);
    setcontext(&(manager_thread->context));
}

void dccthread_wait(dccthread_t *tid)
{
    current_thread->waiting_for = tid;

}

dccthread_t * dccthread_self(void)
{
    return current_thread;
}

const char * dccthread_name(dccthread_t *tid)
{
    return tid->name;
}
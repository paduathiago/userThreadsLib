#include "dccthread.h"
#include "dlist.h"

#include <malloc.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ucontext.h>

#define SIGSTKSZ 8192
#define TIMER_SIGNAL SIGUSR1
#define _XOPEN_SOURCE 700

struct dlist *ready_threads_list;

struct sigevent sev;
struct itimerspec its;
struct sigaction sa;

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

int find_item_dlist(dccthread_t* item, struct dlist* dlist) {
    if (dlist->count == 0) 
        return 0;

    struct dnode* current_item = dlist->head;
    while (current_item != NULL)
    {
        if(current_item->data == item)
            return 1;
        current_item = current_item->next;
    }

    return 0;
}

void timed_preemption()
{   
    timer_t timer;
    // Configure the timer signal handler
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = dccthread_yield;
    sigemptyset(&sa.sa_mask);
    if (sigaction(TIMER_SIGNAL, &sa, NULL) == -1) 
    {
            perror("sigaction");
            exit(1);
    }

    // Configure the timer expiration notification
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = TIMER_SIGNAL;
    sev.sigev_value.sival_ptr = &timer;
    // Create the timer
    if (timer_create(CLOCK_MONOTONIC, &sev, &timer) == -1) 
    {
        perror("timer_create");
        exit(1);
    }

    // Set the timer to expire in 10ms
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 10000000; // 10ms
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    if (timer_settime(timer, 0, &its, NULL) == -1) 
    {
        perror("timer_settime");
        exit(1);
    }
}

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

    timed_preemption();
    
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
    dlist_push_right(ready_threads_list, current_thread);
    swapcontext(&current_thread->context, &manager_thread->context);
}

dccthread_t * dccthread_self(void)
{
    return current_thread;
}

const char * dccthread_name(dccthread_t *tid)
{
    return tid->name;
}
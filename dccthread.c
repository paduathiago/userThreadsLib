#include "dccthread.h"
#include "dlist.h"

#include <malloc.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ucontext.h>

#define TIMER_SIGNAL SIGUSR1
#define _XOPEN_SOURCE 700

struct dlist *ready_threads_list;

ucontext_t manager_context;
dccthread_t *current_thread;

typedef struct dccthread
{
    char *name;
	ucontext_t context;
    dccthread_t *waiting_for;
}dccthread_t;

typedef struct timer_data{
    timer_t timer;
    struct sigevent event;
    struct itimerspec delta;
    struct sigaction action;
} timer_data_t;

timer_data_t preemption_timer;


void new_thread_stack(dccthread_t *thread)
{
    thread->context.uc_stack.ss_sp = malloc(SIGSTKSZ);
    thread->context.uc_stack.ss_size = SIGSTKSZ;
    thread->context.uc_stack.ss_flags = 0;
}

int dlist_find(dccthread_t* item, struct dlist* dlist) {
    struct dnode* node = dlist->head;
    while (node != NULL)
    {
        if(node->data == item)
            return 1;
        node = node->next;
    }
    return 0;
}

void _preemption(int _)
{
    dccthread_yield();
}

void timed_preemption()
{   
    // Configure the timer signal handler
    preemption_timer.action.sa_flags = 0;
    preemption_timer.action.sa_handler = _preemption;
    sigemptyset(&preemption_timer.action.sa_mask);
    if (sigaction(TIMER_SIGNAL, &preemption_timer.action, NULL) == -1) 
    {
        perror("sigaction");
        exit(1);
    }

    // Configure the timer expiration notification
    preemption_timer.event.sigev_notify = SIGEV_SIGNAL;
    preemption_timer.event.sigev_signo = TIMER_SIGNAL;
    preemption_timer.event.sigev_value.sival_ptr = &preemption_timer.timer;
    // Create the timer
    if (timer_create(CLOCK_PROCESS_CPUTIME_ID, &preemption_timer.event, &preemption_timer.timer) == -1) 
    {
        perror("timer_create");
        exit(1);
    }

    // Set the timer to expire in 10ms
    preemption_timer.delta.it_value.tv_sec = 0;
    preemption_timer.delta.it_value.tv_nsec = 10000000; // 10ms
    preemption_timer.delta.it_interval.tv_sec = 0;
    preemption_timer.delta.it_interval.tv_nsec = 10000000;

    if (timer_settime(preemption_timer.timer, 0, &preemption_timer.delta, NULL) == -1) 
    {
        perror("timer_settime");
        exit(1);
    }
}

void dccthread_init(void (*func)(int), int param)
{
    ready_threads_list = dlist_create();
    
    dccthread_create("main", func, param); // create main thread
    
    /* manager_thread = (dccthread_t *) malloc(sizeof(dccthread_t));
    manager_thread -> name = (char *) malloc(sizeof(char) * strlen("manager"));
    manager_thread -> name = "manager"; */
    getcontext(&(manager_context));
    manager_context.uc_link = NULL;
    //new_thread_stack(manager_thread);

    timed_preemption();
    
    while (!dlist_empty(ready_threads_list))
    {
        current_thread = (dccthread_t *) ready_threads_list->head->data;
        swapcontext(&(manager_context), &(current_thread->context));

        dlist_pop_left(ready_threads_list);
    }

    exit(EXIT_SUCCESS);
}

dccthread_t * dccthread_create(const char *name, void (*func)(int), int param)
{
    dccthread_t *new_thread = (dccthread_t *) malloc(sizeof(dccthread_t));
    new_thread->name = (char *) malloc(sizeof(char) * strlen(name));
    strcpy(new_thread->name, name);
    new_thread->waiting_for = NULL;
    
    getcontext(&(new_thread->context));
    new_thread->context.uc_link = &(manager_context);
    new_thread_stack(new_thread);
    dlist_push_right(ready_threads_list, new_thread);
    makecontext(&(new_thread->context), (void (*)())func, 1, param);
    return new_thread;
}

void dccthread_yield(void)
{
    dlist_push_right(ready_threads_list, current_thread);	
	swapcontext(&(current_thread->context), &(manager_context));
}

void dccthread_exit(void)
{
    free(current_thread);
    setcontext(&(manager_context));
}

void dccthread_wait(dccthread_t *tid)
{
    current_thread->waiting_for = tid;
    dlist_push_right(ready_threads_list, current_thread);
    swapcontext(&current_thread->context, &manager_context);
}

dccthread_t * dccthread_self(void)
{
    return current_thread;
}

const char * dccthread_name(dccthread_t *tid)
{
    return tid->name;
}
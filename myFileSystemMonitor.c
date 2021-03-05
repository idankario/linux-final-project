#include "pch.h"
#include "inotify.c"
#include "telnet.c"

#define _GNU_SOURCE

void *backtrace_buffer[BACKTRACE_LENGTH];


//Global vars
sem_t telnet_sem;
void *backtrace_buffer[BACKTRACE_LENGTH];
pthread_t thread_inotify;
pthread_t thread_telnet;
backtrace_s bt;


void  __attribute__ ((no_instrument_function)) reset_backtrace () {
    free(bt.trace);
    bt.trace = (char**)malloc(0*sizeof(char*));
    bt.trace_count = 0;
    bt.is_active = 0;
}
void  __attribute__ ((no_instrument_function)) collect_backtrace  (int trace_count, char** string) {
    bt.trace = (char**)realloc(bt.trace, (bt.trace_count + trace_count) * sizeof(char*));
    for (int h = 0; h < trace_count; h++) {
        bt.trace[bt.trace_count + h] = (char*)malloc(TRACE_LINE_LENGTH*sizeof(char));
        strcpy(bt.trace[bt.trace_count + h], string[h]);
    }
    bt.trace_count += trace_count;
}

//Instrumentation
void  __attribute__ ((no_instrument_function))  __cyg_profile_func_enter (void *this_fn,
                                                                          void *call_site)
{
    if(bt.is_active == 1) {
        //Waits for libcli to finish print
        sem_wait(&telnet_sem);
        reset_backtrace();
    }
    /* Compare two thread identifiers.  */
    if (!pthread_equal(thread_telnet, pthread_self())) {
        int trace_count = backtrace(backtrace_buffer, BACKTRACE_LENGTH);
        char** string = backtrace_symbols(backtrace_buffer, trace_count);
        collect_backtrace(trace_count, string);
    }

}


//Fills execution arguments
void set_opt(int argc, char **argv, parameters* p) {
    int opt;
    while ((opt = getopt(argc, argv, "d:i:")) != -1)
    {
        //copy directory to be watch 
        if(opt=='d') 
        {
            p->directory_to_be_watched = (char*)malloc(128*sizeof(char));
            strcpy(p->directory_to_be_watched, optarg);
        }
        //copy ip  
        else if(opt=='i')
        {
            p->ip_address = (char*)malloc(128*sizeof(char));
            strcpy(p->ip_address, optarg);
        }
        else
            exit(EXIT_FAILURE);
    }
    
}

//SIGINT, SIGABRT handler
void sig_handler(int sig) {
    /* Close descriptor for named semaphore SEM.  */
    sem_close (&telnet_sem);
    sig_handler_inotify(sig);
    exit(0);
}

  parameters p;
int main(int argc, char **argv) {
 	if (argc < 2) {
		printf("Usage: %s PATH [PATH ...]\n", *(argv));
		exit(EXIT_FAILURE);
	}
    //Create web html for nonify
    create_html();

    signal(SIGINT, sig_handler);
    signal(SIGABRT, sig_handler);
    
    /* Initialize semaphore object SEM to VALUE.  If PSHARED then share it with other processes.  */
    if (sem_init(&telnet_sem, 0, 0) == -1){
        printf("sem_init failed\n");
        return 1;
    }

    //Fills execution arguments
    set_opt(argc, argv, &p);

    
    //Creates threads
    if (pthread_create(&thread_inotify, NULL, prepare_for_polling, (void*)&p))
    {
         perror("Failed to create thread inotify\n"); 
        return 1;
    }
    if (pthread_create(&thread_telnet, NULL, init_telnet_thread, (void*)&bt))
    {
         perror("Failed to create thread telnet\n"); 
        return 1;
    }
    //Waits for thread to finish
    pthread_join(thread_inotify, NULL);
    pthread_join(thread_telnet, NULL);

    //Close semaphore
    sem_close (&telnet_sem);

    //Exits from program
	printf("Listening for events stopped.\n");

	/* Close inotify file descriptor */
	close(fd);

	exit(EXIT_SUCCESS);
}








 

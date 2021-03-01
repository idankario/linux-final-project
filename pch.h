#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <execinfo.h>
#include <semaphore.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <libcli.h>
#include <errno.h>
#include <limits.h>
#include <strings.h>
#define INDEX_FILE_LOCATION "/var/www/html/index.html"
#define BACKTRACE_LENGTH 128
#define TRACE_LINE_LENGTH 128
#define MAX_EVENTS 1024  
#define LEN_NAME 25 
#define IP_MAX_LEN 16
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) 
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME ))
#define TIME_SIZE 50
#define PORT 1234
#define MAXLINE 1000
#ifndef PCH_H
#define PCH_H
sem_t telnet_sem;
pthread_t thread_inotify;
pthread_t thread_telnet;
// UDP Client struct
typedef struct socketclient {
    struct sockaddr_in servaddr;
    int sockfd;
} socket_client;
typedef struct backtrace {
    char **trace;
    int trace_count;
    char is_active;
} backtrace_s;
typedef struct parameters
{
  char* directory_to_be_watched;
  char* ip_address;
} parameters;
typedef char* srtP;
//create_socket_client
void create_socket_client(char *ip, socket_client* uc)
{
    // clear servaddr
    bzero(&uc->servaddr, sizeof(uc->servaddr));
    uc->servaddr.sin_addr.s_addr = inet_addr(ip);
    uc->servaddr.sin_port = htons(PORT);
    uc->servaddr.sin_family = AF_INET;
    // create UDP Socket
    uc->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
}

//Sends message provided to udp client provided
void send_message_to_udp_client (socket_client* uc, char* message) {

    // connect to server
    if(connect(uc->sockfd, (struct sockaddr *)&uc->servaddr, sizeof(uc->servaddr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        exit(0);
    }

    sendto(uc->sockfd, message, MAXLINE, 0, (struct sockaddr*)NULL, sizeof(uc->servaddr));
}
void set_current_time(char *ctime_string) {
    /* Returned by `time'.  */
    time_t time_c;
    /* Return the current time and put it in *TIMER if TIMER is not NULL.  */
    time(&time_c); 
    /* ISO C `broken-down time' structure.  */   
    struct tm *brocke_time;
    /* Return the `struct tm' representation of *TIMER in the local timezone.  */
    brocke_time = localtime(&time_c);
    /* Format TP into S according to FORMAT.
   Write no more than MAXSIZE characters and return the number
   of characters written, or 0 if it would exceed MAXSIZE.  */
    strftime(ctime_string, TIME_SIZE, "%d %B %Y: %H:%M", brocke_time);
}

// Resets index file for new sessions
void create_html(){
    FILE *index_file = fopen(INDEX_FILE_LOCATION, "w");
    fprintf(index_file, "<head><title>Unix notify directory watch project unix</title></head>");
    fprintf(index_file, "<h1 styele>Welcome to notify watcher</h1>");
    fclose(index_file);
}
void notify_event_to_web(char *file_name, char *event_name, char *event_ctime) {
    FILE *index_file = fopen(INDEX_FILE_LOCATION, "a+");
    fprintf(index_file, "<h2> FILED ACCESSED: %s</h2> <h3> ACCESS: %s </h3><label>TIME OF ACCESS: %s </label>", file_name, event_name,event_ctime);
    fclose(index_file);
}

#endif
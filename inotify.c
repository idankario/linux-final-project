#include "pch.h"
int fd, wd;
//save udp in struct
socket_client uc;

// Inits inotify thread
void* prepare_for_polling (void *argv)
{
    char* directory_w = ((parameters *)argv)->directory_to_be_watched;
    char* ip_s = ((parameters *)argv)->ip_address;
    
    /* Create and initialize inotify instance.  */
    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
		perror("eror init1");
		exit(EXIT_FAILURE);
	}
  /*  wd = calloc((void*)argv, sizeof(int));*/
	if (wd >0) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}
    create_socket_client(ip_s, &uc);
   /* Do the file control operation described by CMD on FD. The remaining arguments are interpreted depending on CMD.
   This function is a cancellation point and therefore not marked with
   __THROW.   */
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) 
        exit(2);

    /* Add watch of object NAME to inotify instance FD.  Notify about
   events specified by MASK.  */
    wd = inotify_add_watch(fd, directory_w, IN_MODIFY | IN_ACCESS);
    if (wd >0) {
        printf("Erorr could not find this: %s\n", directory_w);
    } else {
        printf("Watching : %s\n", directory_w);
    }

    while (1) {
        char buf[BUF_LEN];
        char message[BUF_LEN];
        int i=0,len=read(fd, buf, BUF_LEN);
        struct inotify_event *event = (struct inotify_event *) &buf[i];
    	
       while(i<len)
       {
            if (event->len) {
                char ctime_string[TIME_SIZE];
                set_current_time(ctime_string);
                /* Print event type */
                /* Event occurred against dir.  */
                if(!(event->mask & IN_ISDIR)) {
                    /* Writtable file was closed.  */
                    if (event->mask & IN_CLOSE_WRITE)
                    {
                         notify_event_to_web(event->name, "IN_CLOSE_WRITE", ctime_string);
                        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n", event->name, "IN_CLOSE_WRITE", ctime_string );
                        send_message_to_udp_client(&uc, message);
                 
                    }
                    /* Unwrittable file closed.  */
                    else if (event->mask & IN_CLOSE_NOWRITE)
                    {
                        notify_event_to_web(event->name, "IN_CLOSE_WRITE", ctime_string);
                        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n", event->name, "IN_CLOSE_WRITE", ctime_string );
                        send_message_to_udp_client(&uc, message);

                    }
                    /* File was opened.  */
                    else if (event->mask & IN_OPEN)
                    {
                         notify_event_to_web(event->name, "IN_OPEN.", ctime_string);
                        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n", event->name, "IN_OPEN", ctime_string );
                        send_message_to_udp_client(&uc, message);
                 
                    }
                    /* File was modified.  */
                    else if (event->mask & IN_MODIFY) {
                        //file_name, event_name,event_ctime
                        notify_event_to_web(event->name, "modified", ctime_string);
                        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n", event->name, "WRITE", ctime_string );
                        send_message_to_udp_client(&uc, message);
                        // File was accessed.
                    } else if (event->mask & IN_ACCESS) {
                        notify_event_to_web(event->name, "read", ctime_string);
                        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n",
                                 event->name, "READ", ctime_string );
                        send_message_to_udp_client(&uc, message);
                    }
                }

            }
           
        }

    }

}

void sig_handler_inotify(int sig) {
    /* Remove the watch specified by WD from the inotify instance FD.  */
    inotify_rm_watch(fd, wd);
    /* Close the file descriptor FD. this function is a cancellation point and therefore not marked with __THROW.  */
    close(uc.sockfd);
    /* Close the file descriptor FD. This function is a cancellation 
    point and therefore not marked with __THROW.  */
    close(fd);
}

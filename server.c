#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <stdbool.h>
#include "socket.h"
#include "client-info.h"


int main(void)
{
    int server_fd, client_fd, epoll_fd;
    int nfds;
    struct epoll_event events[MAX_EVENTS];

    server_fd = setup_socket_server();
    
    epoll_fd = setup_epoll(server_fd);

    printf("Server start on port %d ... \n", PORT);
        
    while(1)
    {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1)
            handle_error("epoll_wait");

        for (int n = 0; n < nfds; n++) {
            if (events[n].data.fd == server_fd) {   // New client
                handle_new_client(server_fd, epoll_fd);
            } else {                                // Existing client
                client_fd = events[n].data.fd;
                handle_exist_client(client_fd);
            }
        }
    }
    close(epoll_fd);
    close(server_fd);

    return 0;
}

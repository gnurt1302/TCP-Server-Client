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
#include <openssl/ssl.h>
#include "socket.h"
#include "client-info.h"
#include "openssl-util.h"

int main(void)
{
    int server_fd, client_fd, epoll_fd;
    int nfds;
    struct epoll_event events[MAX_EVENTS];

    init_openssl();
    SSL_CTX *context = create_context("certs/server.crt", "certs/server.key");
    if (!context) {
        fprintf(stderr, "Failed to create SSL context.\n");
        return 1;
    }

    server_fd = setup_socket_server(); 
    epoll_fd = setup_epoll(server_fd);

    printf("Server start on port %d ... \n", PORT);
        
    while(1)
    {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1)
            handle_error("epoll_wait");

        for (int n = 0; n < nfds; n++) {
            if (events[n].data.fd == server_fd) {
                handle_new_client(server_fd, epoll_fd, context);
            } else {
                client_fd = events[n].data.fd;
                client_t *client = find_client_by_fd(client_fd);
                if (client) {
                    handle_exist_client(client, epoll_fd);
                }
            }
        }
    }
    cleanup_openssl();
    close(epoll_fd);
    close(server_fd);

    return 0;
}

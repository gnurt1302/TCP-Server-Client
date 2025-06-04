#ifndef _SOCKET_H
#define _SOCKET_H

#include <stdbool.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "common.h"
#include "client-info.h"

#define PORT        2000

// #define MAX_CLIENTS 10
// #define MAX_EVENTS  3

int setup_socket_server(void);
int setup_socket_client(void);
int setup_epoll(int server_fd);
void handle_new_client(int server_fd, int epoll_fd, SSL_CTX *context);
void handle_exist_client(client_t *client, int epoll_fd);

#endif  // _SOCKET_H
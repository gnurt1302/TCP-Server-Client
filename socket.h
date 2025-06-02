#ifndef _SOCKET_H
#define _SOCKET_H

#include <stdbool.h>
#include "common.h"

#define PORT        2000

// #define MAX_CLIENTS 10
// #define MAX_EVENTS  3

int setup_socket_server(void);
int setup_socket_client(void);
int setup_epoll(int server_fd);
void handle_new_client(int server_fd, int epoll_fd);
void handle_exist_client(int client_fd);

bool server_authenticate(int client_fd);
bool client_authenticate(int client_fd);

#endif  // _SOCKET_H
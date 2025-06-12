#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <sys/epoll.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "socket.h"
#include "client-info.h"
#include "common.h"
#include "auth.h"

int setup_socket_server(void)
{
    int server_fd;
    int opt = 1;
    struct sockaddr_in  server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
        handle_error("socket");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
        handle_error("setsockopt");

    server_addr.sin_family		= AF_INET;
    server_addr.sin_port		= htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        handle_error("bind");

    if (listen(server_fd, MAX_CLIENTS) == -1)
        handle_error("listen");

    return server_fd;
}

int setup_socket_client(void) 
{
    int client_fd;
    struct sockaddr_in server_addr;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
        handle_error("socket");

    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) == -1)
        handle_error("inet_pton");

    if(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        handle_error("connect");

    printf("Connected to server\n");
    return client_fd;
}

int setup_epoll(int server_fd)
{
    int epoll_fd;
    struct epoll_event ev;

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) 
        handle_error("epoll_create1");

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) 
        handle_error("epoll_ctl");

    return epoll_fd;
}

void handle_new_client(int server_fd, int epoll_fd, SSL_CTX *context)
{
    int client_fd;
    struct sockaddr_in  client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    struct epoll_event ev;
    char username[BUFF_SIZE];

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_fd == -1)
        handle_error("accept");

    // Create SSL connection for new client
    SSL *ssl = SSL_new(context);
    SSL_set_fd(ssl, client_fd);
    
    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(client_fd);
        return;
    }

    // Authenticate client 
    if (!server_authenticate(ssl, username)) {
        printf("Authentication failed. Client disconnected.\n");
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client_fd);
        return;
    }

    add_client(client_fd, ssl, username);

    // Epoll 
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
        handle_error("epoll_ctl");
    }

    printf("Username %s connected \n", get_username(client_fd));
}

void handle_exist_client(client_t *client, int epoll_fd)
{
    char message[2100];
    int byte_recv = SSL_read(client->ssl, buffer, BUFF_SIZE - 1);

    if (byte_recv > 0) {  // Client sent message
        buffer[byte_recv] = '\0';
        printf("[%s]: %s\n", client->username, buffer);

        // Echo back to client
        // SSL_write(client->ssl, buffer, byte_recv);
        snprintf(message, sizeof(message), "[%s]: %s\n", client->username, buffer);
        broadcast_message(message);

    } else {
        int err = SSL_get_error(client->ssl, byte_recv);
        if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL) {
            printf("Username %s disconnected.\n", client->username);

            // Remove from epoll
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->fd, NULL);
            remove_client(client);
        } else {
            ERR_print_errors_fp(stderr);
        }
    }
}
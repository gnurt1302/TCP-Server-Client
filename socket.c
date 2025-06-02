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
#include <arpa/inet.h>
#include "socket.h"
#include "client-info.h"
#include "common.h"

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

void handle_new_client(int server_fd, int epoll_fd)
{
    int client_fd;
    struct sockaddr_in  client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    struct epoll_event ev;

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_fd == -1)
        handle_error("accept");

    bool is_authenticated = server_authenticate(client_fd);
    if (!is_authenticated) {
        printf("Authentication failed. Client disconnected.\n");
        return; 
    }

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
        handle_error("epoll_ctl");
    }

    printf("User name %s connected \n", get_username(client_fd));
}

void handle_exist_client(int client_fd)
{
    char buffer[BUFF_SIZE];

    int byte_recv = recv(client_fd, buffer, BUFF_SIZE, 0);
    
    if (byte_recv > 0) {                // Client sent
        buffer[byte_recv] = '\0';
        send(client_fd, buffer, byte_recv, 0);

        printf("[%s]: %s", get_username(client_fd), buffer);

    } else {                            // Client disconnect
        printf("User %s disconnected.\n", get_username(client_fd));
        remove_client(client_fd);
        close(client_fd);
    }

}

bool server_authenticate(int client_fd)
{
    char username[BUFF_SIZE];
    char password[BUFF_SIZE];

    const char *auth_req_msg = "Please login (username password)\n";
    const char *auth_success_msg = "Login success!!!\n";
    const char *auth_failed_msg = "Login failed!!!\n";

    int auth_status = 0;

    send(client_fd, auth_req_msg, strlen(auth_req_msg), 0);

    // Get login information
    int byte_recv = recv(client_fd, buffer, BUFF_SIZE, 0);
    if (byte_recv > 0) {                
        buffer[byte_recv] = '\0';
    }
    snprintf(username, sizeof(username), "%s", buffer);
    username[strcspn(username, "\r\n")] = '\0';

    byte_recv = recv(client_fd, buffer, BUFF_SIZE, 0);
    if (byte_recv > 0) {                
        buffer[byte_recv] = '\0';
    }
    snprintf(password, sizeof(password), "%s", buffer);
    password[strcspn(password, "\r\n")] = '\0';

    if ((strcmp(username, "admin") == 0 && strcmp(password, "1") == 0) ||
        (strcmp(username, "trung") == 0 && strcmp(password, "1") == 0) || 
        (strcmp(username, "gnurt") == 0 && strcmp(password, "1") == 0) )  {
            send(client_fd, auth_success_msg, strlen(auth_success_msg), 0);

            add_client(client_fd, username);
            return true;
    } else {
        send(client_fd, auth_failed_msg, strlen(auth_failed_msg), 0);
        close(client_fd);
        return false;
    }
}

bool client_authenticate(int client_fd)
{
    char username[BUFF_SIZE];
    char password[BUFF_SIZE];
    char auth_result_msg[BUFF_SIZE];
    int byte_recv;

    byte_recv = recv(client_fd, buffer, BUFF_SIZE, 0);
    if (byte_recv > 0) {                // Printf: Please login
        buffer[byte_recv] = '\0';
        printf("%s", buffer);
    }

    printf("Username: ");
    fflush(stdout);
    fgets(username, sizeof(username), stdin);

    printf("Password: ");
    fflush(stdout);
    fgets(password, sizeof(password), stdin);

    send(client_fd, username, sizeof(username), 0);
    send(client_fd, password, sizeof(password), 0);

    byte_recv = recv(client_fd, buffer, BUFF_SIZE, 0);
    if (byte_recv > 0) {
        buffer[byte_recv] = '\0';

        snprintf(auth_result_msg, sizeof(auth_result_msg), "%s", buffer);
        auth_result_msg[strcspn(auth_result_msg, "\n")] = 0;

        printf("%s", auth_result_msg);

        if (strstr(auth_result_msg, "success") != NULL)
            return true;
    }
    return false;
    
}
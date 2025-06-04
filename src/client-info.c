#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "client-info.h"
#include "common.h"

client_t *clients = NULL;
int client_count = 0;
int client_capacity = 0;

void add_client(int client_fd, SSL *ssl, const char *username) 
{
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].fd == client_fd) return;
    }

    if (client_count == client_capacity) {
        int new_capacity = (client_capacity == 0) ? 10 : client_capacity * 2;
        client_t *new_clients = realloc(clients, new_capacity * sizeof(client_t));
        if (!new_clients) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        clients = new_clients;
        client_capacity = new_capacity;
    }
    
    clients[client_count].fd = client_fd;
    clients[client_count].ssl = ssl;
    strncpy(clients[client_count].username, username, BUFF_SIZE - 1);
    clients[client_count].username[BUFF_SIZE - 1] = '\0';
    clients[client_count].is_authenticated = true;
    client_count++;
    
}

void remove_client(int client_fd) 
{
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].fd == client_fd) {
            SSL_shutdown(clients[i].ssl);
            SSL_free(clients[i].ssl);
            close(clients[i].fd);

            // Ghi đè phần tử bị xóa bằng phần tử cuối
            clients[i] = clients[client_count - 1];
            client_count--;
            return;
        }
    }
}

const char* get_username(int client_fd) 
{
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].fd == client_fd) {
            return clients[i].username;
        }
    }
    return "Unknown";
}


void print_client(void) 
{
    printf("Connected Clients\n");
    for (int i = 0; i < client_count; ++i) {
        printf("FD: %d, Username: %s, Auth: %s\n",
            clients[i].fd,
            clients[i].username,
            clients[i].is_authenticated ? "Yes" : "No");
    }
}

client_t *find_client_by_fd(int client_fd) 
{
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].fd == client_fd) {
            return &clients[i];
        }
    }
    return NULL;
}
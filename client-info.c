#include <stdio.h>
#include <string.h>
#include "client-info.h"
#include "common.h"

client_t clients[MAX_CLIENTS] = {0};
int client_count = 0;

void add_client(int client_fd, const char *username) 
{
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].fd == client_fd) return;
    }

    if (client_count < MAX_CLIENTS) {
        clients[client_count].fd = client_fd;
        strncpy(clients[client_count].username, username, BUFF_SIZE - 1);
        clients[client_count].username[BUFF_SIZE - 1] = '\0';
        clients[client_count].is_authenticated = true;
        client_count++;
    }
}

void remove_client(int client_fd) {
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].fd == client_fd) {
            // Dịch các client phía sau lên
            printf("Removing username: %s\n", clients[i].username);
            for (int j = i; j < client_count - 1; ++j) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
}

const char* get_username(int client_fd) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].is_authenticated && clients[i].fd == client_fd) {
            return clients[i].username;
        }
    }
    return "Unknown";
}
#ifndef _CLIENT_INFO_H
#define _CLIENT_INFO_H

#include <stdbool.h>
#include "common.h"

#define MAX_CLIENTS 10

typedef struct {
    int fd;
    char username[BUFF_SIZE];
    bool is_authenticated;
} client_t;

extern client_t clients[MAX_CLIENTS];
extern int client_count;

void add_client(int client_fd, const char *username);
void remove_client(int client_fd);
const char* get_username(int client_fd);

#endif // _CLIENT_INFO_H

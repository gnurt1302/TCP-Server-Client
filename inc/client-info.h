#ifndef _CLIENT_INFO_H
#define _CLIENT_INFO_H

#include <stdbool.h>
#include <openssl/ssl.h>
#include "common.h"

#define MAX_CLIENTS 10

typedef struct {
    int fd;
    SSL *ssl;
    bool is_authenticated;
    char username[BUFF_SIZE];
} client_t;

extern client_t *clients;
extern int client_count;
extern int client_capacity;

void add_client(int client_fd, SSL *ssl, const char *username);
void remove_client(int client_fd);
const char* get_username(int client_fd);
void print_client(void);
client_t *find_client_by_fd(int client_fd);

#endif // _CLIENT_INFO_H

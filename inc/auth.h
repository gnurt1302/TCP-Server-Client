#ifndef _AUTH_H
#define _AUTH_H

#include <stdbool.h>

#define USER_FILE "users.txt"

bool server_authenticate(SSL *ssl, char *out_username);
bool client_authenticate(SSL *ssl);

#endif  // _AUTH_H
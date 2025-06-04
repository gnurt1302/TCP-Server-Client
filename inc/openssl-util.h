#ifndef _OPENSSL_UTIL_H
#define _OPENSSL_UTIL_H

#include <openssl/ssl.h>

void init_openssl(void);
void cleanup_openssl();
SSL_CTX *create_context(const char *cert_file, const char *key_file);

#endif  // _OPENSSL_UTIL_H
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "socket.h"
#include "client-info.h"
#include "auth.h"
#include "openssl-util.h"

int main(void) 
{
    char message[BUFF_SIZE];

    init_openssl();
    SSL_CTX *context = create_context("certs/server.crt", NULL);
    if (!context) {
        fprintf(stderr, "Failed to create SSL context.\n");
        return 1;
    }

    int client_fd = setup_socket_client();

    SSL *ssl = SSL_new(context);
    SSL_set_fd(ssl, client_fd);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        SSL_CTX_free(context);
        close(client_fd);
        return 1;
    }

    if (!client_authenticate(ssl)) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(context);
        close(client_fd);
        return 0;
    }

    while (1) {
        printf("Enter message (type /exit to exit): ");
        fflush(stdout);
        fgets(message, sizeof(message), stdin);
        
        SSL_write(ssl, message, strlen(message));

        int byte_recv = SSL_read(ssl, buffer, BUFF_SIZE - 1);
        if (byte_recv > 0) { 
            buffer[byte_recv] = '\0';
            printf("Update from server: %s", buffer);
        } else {
            printf("Server disconnected.\n");
            break;
        }
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(context);
    close(client_fd);
    close(client_fd);
    return 0;
}
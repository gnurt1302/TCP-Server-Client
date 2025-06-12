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

    // Initialize OpenSSL
    init_openssl();

    // Create SSL context for client
    SSL_CTX *context = create_client_context("certs/server.crt", "certs/server.key");
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
        // Get message from user
        printf("Enter message (type /exit to exit): ");
        fflush(stdout);
        fgets(message, sizeof(message), stdin);

        // Exit if user types /exit
        message[strcspn(message, "\n")] = '\0';
        if (strcmp(message, "/exit") == 0) {
            break;
        }

        // Send message to server
        SSL_write(ssl, message, strlen(message));

        // Receive message from server
        int byte_recv = SSL_read(ssl, buffer, BUFF_SIZE - 1);
        if (byte_recv > 0) { 
            buffer[byte_recv] = '\0';
            printf("%s\n", buffer);
        } else {
            printf("recv from server failed\n");
            break;
        }
    }

    printf("Client disconnected.\n");
    // Cleanup
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(context);
    close(client_fd);

    return 0;
}
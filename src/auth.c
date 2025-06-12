#include <stdio.h> 
#include <stdlib.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdbool.h>
#include "socket.h"
#include "client-info.h"
#include "common.h"
#include "auth.h"

bool server_authenticate(SSL *ssl, char *out_username)
{
    char username[BUFF_SIZE];
    char password[BUFF_SIZE];
    char file_username[BUFF_SIZE];
    char file_password[BUFF_SIZE];
    int byte_recv;

    const char *auth_req_msg = "Please login (username password)\n";
    const char *auth_success_msg = "Login success!!!\n";
    const char *auth_failed_msg = "Login failed!!!\n";

    // Send request login 
    SSL_write(ssl, auth_req_msg, strlen(auth_req_msg));

    // Get login information (username, password)
    byte_recv = SSL_read(ssl, buffer, BUFF_SIZE - 1);
    if (byte_recv <= 0) return false;
    buffer[byte_recv] = '\0';
    snprintf(username, sizeof(username), "%s", buffer);
    username[strcspn(username, "\r\n")] = '\0';

    byte_recv = SSL_read(ssl, buffer, BUFF_SIZE - 1);
    if (byte_recv <= 0) return false;
    buffer[byte_recv] = '\0';
    snprintf(password, sizeof(password), "%s", buffer);
    password[strcspn(password, "\r\n")] = '\0';

    // Open user.txt file
    FILE *fp;
    fp = fopen(USER_FILE, "r");
    if (fp == NULL) {
        handle_error("open users.txt");
    }

    // Compare login information with user file
    while (fscanf(fp, "%s %s", file_username, file_password) != EOF) {
        if (strcmp(username, file_username) == 0 && strcmp(password, file_password) == 0) {
            // Authentication success
            SSL_write(ssl, auth_success_msg, strlen(auth_success_msg));
            snprintf(out_username, BUFF_SIZE, "%s", username);
            fclose(fp);
            return true;
        }
    }
    // Authentication failed
    SSL_write(ssl, auth_failed_msg, strlen(auth_failed_msg));
    fclose(fp);
    return false;
}

bool client_authenticate(SSL *ssl)
{
    char username[BUFF_SIZE];
    char password[BUFF_SIZE];
    char auth_result_msg[BUFF_SIZE];
    int byte_recv;

    // Receive request information
    byte_recv = SSL_read(ssl, buffer, BUFF_SIZE - 1);
    if (byte_recv <= 0) return false;
    buffer[byte_recv] = '\0';
    printf("%s", buffer);

    // Get login information from user
    printf("Username: ");
    fflush(stdout);
    fgets(username, sizeof(username), stdin);

    printf("Password: ");
    fflush(stdout);
    fgets(password, sizeof(password), stdin);

    // Send login information to server to authenticate
    SSL_write(ssl, username, strlen(username));
    SSL_write(ssl, password, strlen(password));

    // Get authenticate status from server
    byte_recv = SSL_read(ssl, buffer, BUFF_SIZE - 1);
    if (byte_recv > 0) {
        buffer[byte_recv] = '\0';

        snprintf(auth_result_msg, sizeof(auth_result_msg), "%s", buffer);
        auth_result_msg[strcspn(auth_result_msg, "\n")] = 0;

        printf("%s\n", auth_result_msg);

        if (strstr(auth_result_msg, "success") != NULL)
            return true;
    }
    return false;
    
}
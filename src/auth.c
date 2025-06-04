#include <stdio.h> 
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

    // Compare login information
    if ((strcmp(username, "admin") == 0 && strcmp(password, "1") == 0) ||
        (strcmp(username, "trung") == 0 && strcmp(password, "1") == 0) || 
        (strcmp(username, "gnurt") == 0 && strcmp(password, "1") == 0) )  {
            
        SSL_write(ssl, auth_success_msg, strlen(auth_success_msg));

        if (out_username)
            strncpy(out_username, username, BUFF_SIZE - 1);
        
        return true;
    } else {
        SSL_write(ssl, auth_failed_msg, strlen(auth_failed_msg));
        return false;
    }
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

        printf("%s", auth_result_msg);

        if (strstr(auth_result_msg, "success") != NULL)
            return true;
    }
    return false;
    
}
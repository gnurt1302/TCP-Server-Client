#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "socket.h"
#include "client-info.h"


int main(void) 
{
    char message[BUFF_SIZE];
    int client_fd = setup_socket_client();

    if (!client_authenticate(client_fd)) {
        close(client_fd);
        return 0;
    }
    while (1) {
        printf("Enter message: ");
        fflush(stdout);
        fgets(message, sizeof(message), stdin);
        send(client_fd, message, strlen(message), 0);

        int byte_recv = recv(client_fd, buffer, BUFF_SIZE, 0);
        if (byte_recv > 0) { 
            buffer[byte_recv] = '\0';
            printf("Update from server: %s", buffer);
        } else {
            printf("Server disconnected.\n");
            break;
        }
    }
    close(client_fd);
    return 0;
}


#ifndef _COMMON_H
#define _COMMON_H

#define BUFF_SIZE   1024
#define MAX_CLIENTS 10
#define MAX_EVENTS  3

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

extern char buffer[BUFF_SIZE];

#endif  // _COMMON_H
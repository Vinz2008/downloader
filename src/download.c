#include "download.h"
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#define MAX_ANSWER_LENGTH (1024 * 1024)


bool download(char* url, char* outfile){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        fprintf(stderr, "ERROR : Can't connect to socket");
        return false;
    }
    struct sockaddr sock_addr;
    struct addrinfo* addr_info;

    //struct addrinfo hints; // TODO
    const char* service = "80";
    int res_addrinfo = getaddrinfo(url, service, /*&hints*/ NULL, &addr_info);
    if (res_addrinfo != 0){
        fprintf(stderr, "ERROR getaddrinfo : %s\n", gai_strerror(res_addrinfo));
    }
    sock_addr = *addr_info->ai_addr;
    if (connect(sock, &sock_addr, sizeof(sock_addr)) == -1){
        fprintf(stderr, "ERROR : Can't connect to socket");
        return false;
    }

    freeaddrinfo(addr_info);

    const char* buf = "GET / HTTP/1.0\r\n\r\n";

    ssize_t bytes_sent = send(sock, buf, strlen(buf), 0);

    char answer_buf[MAX_ANSWER_LENGTH];

    // TODO : parse http header
    
    ssize_t bytes_received = recv(sock, answer_buf, sizeof(answer_buf), 0);

    FILE* f_out = fopen(outfile, "w");
    fwrite(answer_buf, sizeof(char), bytes_received, f_out);
    fclose(f_out);

    return true;
}
#include "download.h"
#include "http_header.h"
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include "util.h"
#include "ssl.h"


#define MAX_ANSWER_LENGTH (10 * 1024 * 1024) // 10 mb


int find_padding_bytes_nb(char* answer_buf, ssize_t bytes_received){
    int pading_bytes_nb = 0;
    int pos = 0;
    while (pos < bytes_received && answer_buf[pos] != '\0'){
        pos++;
    }

    while (pos < bytes_received && answer_buf[pos] == '\0'){
        pading_bytes_nb++;
        pos++;
    }

    return pading_bytes_nb;
}


const char* get_url_without_prefix(const char* url){
    if (startswith(url, "http")){
        url += strlen("http://");
    } else if (startswith(url, "https")){
        url += strlen("https://") + 1;
    }
    if (*url == '/'){
        url++;
    }
    return url;
}

// no prefix, no path but subdomain included (ex : https://www.google.com/a -> www.google.com)
const char* get_url_trimmed(const char* url){
    url = get_url_without_prefix(url);
    size_t url_length = strlen(url);

    int url_host_length = 0;
    printf("url temp : %s\n", url);
    while (url_host_length < url_length && url[url_host_length] != '/' && url[url_host_length] != '\0'){
        url_host_length++;
    }

    printf("url[url_host_length] : %c\n", url[url_host_length]);
    printf("url trimmed temp : %.*s\n", url_host_length, url);


    char* url_host_str = malloc((url_host_length+1) * sizeof(char));
    memcpy(url_host_str, url, url_host_length);
    url_host_str[url_host_length] = '\0';
    return url_host_str;
}

// remove subdomain, protocol, etch
char* get_url_host(const char* url){
    url = get_url_without_prefix(url);
    size_t url_length = strlen(url);
    int dot_nb = 0;
    int i = 0;
    printf("url temp : %s\n", url);
    while (i < url_length && url[i] != '/'){
        if (url[i] == '.'){
            dot_nb++;
        }
        i++;
    }
    int advanced_nb = 0;
    if (dot_nb > 1){
        while (dot_nb > 1){
            while (*url != '.' ){
                url++;
                advanced_nb++;
            }
            url++;
            advanced_nb++;
            dot_nb--;
        }

    } else if (*url == '/'){
        url++;
        advanced_nb++;
    }
    int url_host_length = 0;
    while (url_host_length < url_length-advanced_nb && url[url_host_length] != '/' && url[url_host_length] != '\0'){
        url_host_length++;
    }

    //printf("url temp : %.*s\n", url_host_length, url);

    //printf("url[%d] : %c\n", url_host_length, url[url_host_length]);


    char* url_host_str = malloc((url_host_length+1) * sizeof(char));
    memcpy(url_host_str, url, url_host_length);
    url_host_str[url_host_length] = '\0';
    return url_host_str;
}

const char* get_url_path(const char* url){
    url = get_url_without_prefix(url);

    while (*url != '\0' && *url != '/'){
        url++;
    }

    if (*url == '\0'){
        return "/";
    } else {
        return url;
    }
}

const char* get_port_string(bool is_https){
    if (is_https){
        return "443";
    }

    return "80";
}


bool download(const char* url, const char* outfile){
    bool is_https = startswith(url, "https://");

    SSL* conn = NULL;
    if (is_https){
        printf("use https\n");
        if (!get_ssl_context()){
            init_ssl();
        }
        conn = SSL_new(get_ssl_context());
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        fprintf(stderr, "ERROR : Can't connect to socket");
        return false;
    }

    if (is_https){
        SSL_set_fd(conn, sock);
    }
    
    struct sockaddr sock_addr;
    struct addrinfo* addr_info;

    //struct addrinfo hints; // TODO
    const char* service = get_port_string(is_https);
    char* url_host = get_url_host(url);
    int res_addrinfo = getaddrinfo(url_host, service, /*&hints*/ NULL, &addr_info);
    if (res_addrinfo != 0){
        fprintf(stderr, "ERROR getaddrinfo %s : %s\n", url_host, gai_strerror(res_addrinfo));
        return false;
    }
    sock_addr = *addr_info->ai_addr;
    
    if (connect(sock, &sock_addr, sizeof(sock_addr)) == -1){
        fprintf(stderr, "ERROR : Can't connect to socket");
        return false;
    }

    freeaddrinfo(addr_info);

    if (is_https){
        int err;
        if ((err = SSL_connect(conn)) <= 0){
            fprintf(stderr, "ERROR : Can't create SSL connection (err %x)\n", err);
            log_ssl();
            return false;
        }
    }


    // TODO : use HTTP 1.1
    const char* path = get_url_path(url);
    //const char* send_format = "GET %s HTTP/1.1\nHost: %s\nUser-Agent: download/1.0\nConnection: Keep-Alive\nAccept: */*\r\n\r\n";
    const char* send_format = "GET %s HTTP/1.1\nHost: %s\nUser-Agent: download/1.0\r\n\r\n";

    char* url_trimmed = get_url_trimmed(url);

    size_t send_length = strlen(send_format) + strlen(url_trimmed) + strlen(path) + 1;

    char* send_buf = malloc(send_length * sizeof(char));

    printf("send_length : %ld\n", send_length);

    
    snprintf(send_buf, send_length, send_format, path, url_trimmed);

    printf("request : %s\n", send_buf);

    ssize_t bytes_sent = -1;

    if (is_https){
        bytes_sent = SSL_write(conn, send_buf, send_length);
    } else {
        bytes_sent = send(sock, send_buf, send_length, 0);
    }

    char* answer_buf = malloc(sizeof(char) * MAX_ANSWER_LENGTH);
    char* answer_buf_allocation = answer_buf;

    // TODO : parse http header

    ssize_t bytes_received = 0;

    ssize_t current_chunk_received;

    if (is_https){
        while ((current_chunk_received = SSL_read(conn, answer_buf + bytes_received, MAX_ANSWER_LENGTH)) > 0){
            bytes_received += current_chunk_received;
            /*if (memcmp(answer_buf[bytes_received-2], "\r\n", 2) == 0){
                break;
            }*/
        }

    } else {
        while ((current_chunk_received = recv(sock, answer_buf + bytes_received, MAX_ANSWER_LENGTH, 0)) > 0){
            bytes_received += current_chunk_received;
        
            /*if (memcmp(answer_buf + (bytes_received-2), "\r\n", 2) == 0){
                break;
            }*/
        }
    }

    if (is_https){
        SSL_free(conn);
    }
    

    printf("received %ld bytes\n", bytes_received);

    //printf("answer_buf : %s\n", answer_buf);

    struct http_header* header = parse_header(&answer_buf);

    debug_print_http_header(header);


    bool is_300_code = header->answer_code >= 300 && header->answer_code < 400;
    if (is_300_code || header->answer_code == 201){
        // redirect
        printf("http code %d, redirecting to %s\n", header->answer_code, header->location_redirection);
        bool res = download(header->location_redirection, outfile);
        free_header(header);
        return res;
    }


    free_header(header);

    int pading_bytes_nb = find_padding_bytes_nb(answer_buf, bytes_received);


    FILE* f_out = fopen(outfile, "w");
    fwrite(answer_buf, sizeof(char), bytes_received-pading_bytes_nb, f_out);
    fclose(f_out);

    free(answer_buf_allocation);

    return true;
}
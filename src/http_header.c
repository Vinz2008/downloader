#include "http_header.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

#define MAX_LENGTH_HTTP_VERSION 3 // 1.0 or 2.0 or 3.0, etc so 3 chars long

static float parse_http_version_number(char** buf){
    char http_version_str[MAX_LENGTH_HTTP_VERSION+1];
    int http_version_iter = 0;
    while ((isdigit(**buf) || **buf == '.') && http_version_iter < MAX_LENGTH_HTTP_VERSION){
        http_version_str[http_version_iter] = **buf;
        (*buf)++;
        http_version_iter++;
    }

    http_version_str[http_version_iter] = '\0';

    char *end;
    float version_nb = (int)strtof(http_version_str, &end);

    if ((end != NULL && *end != '\0') || version_nb < 0.9 || version_nb > 2){
        fprintf(stderr, "ERROR: invalid answer from server (invalid http version number)\n");
        exit(1);
    }

    return version_nb;
}

#define MAX_LENGTH_HTTP_CODE 3

static int parse_http_answer_code(char** buf){
    int http_code_iter = 0;
    char http_code_str[MAX_LENGTH_HTTP_VERSION+1];
    while ((isdigit(**buf) || **buf == '.') && http_code_iter < MAX_LENGTH_HTTP_CODE){
        http_code_str[http_code_iter] = **buf;
        (*buf)++;
        http_code_iter++;
    }

    http_code_str[http_code_iter] = '\0';

    char* end;
    int answer_code = strtol(http_code_str, &end, 10);
    if (*end != '\0' || answer_code < 100 || answer_code > 527){
        fprintf(stderr, "ERROR: invalid answer from server (invalid http code)\n");
        exit(1);
    }

    return answer_code;
}

static char* get_string_until(char** buf, char c){
    int length = 0;
    while ((*buf)[length] != '\n'){
        length++;
    }

    char* temp_str = malloc(sizeof(char) * (length+1));
    memcpy(temp_str, *buf, length);
    temp_str[length] = '\0';

    *buf += length;

    *buf += 1; // pass char c

    return temp_str;
}

// TODO : inline them ? (will it not be less clear ?)

static inline char* parse_http_answer_code_name(char** buf){
    return get_string_until(buf, '\n');
}

static inline char* parse_date_string(char** buf){
    return get_string_until(buf, '\n');
}

static inline char* parse_content_type(char** buf){
    return get_string_until(buf, ';');
}

static inline char* parse_content_type_options(char** buf){
    return get_string_until(buf, '\n');
}

static inline char* parse_server_name(char** buf){
    return get_string_until(buf, '\n');
}

static inline char* parse_location(char** buf){
    return get_string_until(buf, '\n');
}

static enum Connection parse_connection(char** buf){
    enum Connection connection = CONNECTION_CLOSE;

    char* connection_str = get_string_until(buf, '\n');
    if (strcmp(connection_str, "close") != 0){
        connection = CONNECTION_CLOSE;
    } else {
        connection = CONNECTION_OTHER;
    }
    free(connection_str);

    return connection;
}

static enum TransferEncoding parse_transfer_encoding(char** buf){
    enum TransferEncoding transfer_encoding = TRANSFER_ENCODING_NONE;


    char* encoding_str = get_string_until(buf, '\n');
    if (strcmp(encoding_str, "chunked") == 0){
        transfer_encoding = TRANSFER_ENCODING_CHUNKED;
    } else {
        transfer_encoding = TRANSFER_ENCODING_OTHER;
    }
    free(encoding_str);

    return transfer_encoding;
}

static void ignore_line(char** buf){
    while (**buf != '\n'){
        *buf += 1;
    }
    *buf += 1; // pass '\n'
}

static const char* ignored_http_header_flag[] = {
    "Vary",
    "Expires",
    "Set-Cookie",
    "Accept-Ranges",
    "Last-Modified",
    "Cache-Control",
    "X-Frame-Options",
    "Referrer-Policy",
    "X-XSS-Protection",
    "X-Content-Type-Options",
    "Content-Security-Policy",
    "Strict-Transport-Security",
    "Content-Security-Policy-Report-Only",
};

bool is_ignored_http_header_flag(char* s){
    for (int i = 0; i < sizeof(ignored_http_header_flag)/sizeof(char*); i++){
        if (startswith(s, ignored_http_header_flag[i])){
            return true;
        }
    }
    return false;
}


// TODO : return a ptr instead ? (too big struct ?)
struct http_header* parse_header(char** buf){
    // first line : HTTP/1.0 200 OK
    const char* http_str = "HTTP/";
    *buf += strlen(http_str);
    float version_number = parse_http_version_number(buf);
    *buf += 1; // skip space

    int answer_code = parse_http_answer_code(buf);

    char* answer_code_name = parse_http_answer_code_name(buf);

    const char* Server_static_str = "Server";
    const char* Date_static_str = "Date";
    const char* Content_Type_static_str = "Content-Type";
    const char* Connection_static_str = "Connection";
    const char* Location_static_str = "Location";
    const char* Transfer_Encoding_static_str = "Transfer-Encoding";

    const int colon_plus_space_length = 2;

    char* date_str = NULL;
    char* content_type = NULL;
    char* content_type_options = NULL;
    char* server_name = NULL;
    char* location_redirection = NULL;

    enum Connection connection = CONNECTION_CLOSE;
    enum TransferEncoding transfer_encoding = TRANSFER_ENCODING_NONE;

    while (**buf != '\0'){
        // TODO : add Content-Length
        if (startswith(*buf, Content_Type_static_str)){
            // line : Content-Type: text/html; charset=utf-8
            *buf += strlen(Content_Type_static_str) + colon_plus_space_length;
            content_type = parse_content_type(buf);
            if (**buf != '\n'){
                content_type_options = parse_content_type_options(buf);
            }

        } else if (startswith(*buf, Server_static_str)){
            // line : Server: gws
            *buf += strlen(Server_static_str) + colon_plus_space_length;
            server_name = parse_server_name(buf);
        } else if (startswith(*buf, Transfer_Encoding_static_str)){
            // line : Transfer-Encoding: chunked
            *buf += strlen(Server_static_str) + colon_plus_space_length;
            transfer_encoding = parse_transfer_encoding(buf);
        } else if (startswith(*buf, Connection_static_str)){
            // line : Connection: close
            *buf += strlen(Connection_static_str) + colon_plus_space_length;
            connection = parse_connection(buf);
        } else if (startswith(*buf, Date_static_str)){
            // line : Date: Sun, 09 Mar 2025 15:15:40 GMT
            *buf += strlen(Date_static_str) + colon_plus_space_length;
            date_str = parse_date_string(buf);
        } else if (startswith(*buf, Location_static_str)){
            // line : Location: https://git.kernel.org/
            *buf += strlen(Location_static_str) + colon_plus_space_length;
            location_redirection = parse_location(buf);
        } else {
            if (is_ignored_http_header_flag(*buf)){
                ignore_line(buf);
            } else {
                break;
            }
        }
    }

    *buf += 2; // pass '\n' so the file starts at first line

    struct http_header header = (struct http_header){
        .version_number = version_number,
        .answer_code = answer_code,
        .answer_code_name = answer_code_name,
        .date_str = date_str,
        .content_type = content_type,
        .content_type_options = content_type_options,
        .server_name = server_name,
        .location_redirection = location_redirection,
        .connection = connection,
    };


    struct http_header* res = malloc(sizeof(struct http_header));
    memcpy(res, &header, sizeof(struct http_header));
    return res;
}

static void free_if_not_null(void* ptr){
    if (ptr){
        free(ptr);
    }
}


void free_header(struct http_header* header){
    free_if_not_null(header->answer_code_name);
    free_if_not_null(header->date_str);

    free_if_not_null(header->content_type);

    free_if_not_null(header->content_type_options);

    free_if_not_null(header->location_redirection);

    free(header);
}

void debug_print_http_header(struct http_header* header){
    printf("http %0.1f : %d %s\n", header->version_number, header->answer_code, header->answer_code_name);
    printf("date : %s\n", header->date_str);
}
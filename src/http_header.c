#include "http_header.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>


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

static char* parse_http_answer_code_name(char** buf){
    int answer_code_name_length = 0;
    while ((*buf)[answer_code_name_length] != '\n'){
        answer_code_name_length++;
    }

    char* answer_code_name = malloc(sizeof(char) * (answer_code_name_length+1));
    memcpy(answer_code_name, *buf, answer_code_name_length);
    answer_code_name[answer_code_name_length] = '\0';

    *buf += answer_code_name_length;

    return answer_code_name;
}

static char* parse_date_string(char** buf){
    int date_str_length = 0;
    while ((*buf)[date_str_length] != '\n'){
        date_str_length++;
    }

    char* date_str = malloc(sizeof(char) * (date_str_length+1));
    memcpy(date_str, *buf, date_str_length);
    date_str[date_str_length] = '\0';

    *buf += date_str_length;

    return date_str;
}

static void ignore_line(char** buf){
    while (**buf != '\n'){
        *buf += 1;
    }
}

// TODO : return a ptr instead ? (too big struct ?)
struct http_header parse_header(char** buf){
    // first line : HTTP/1.0 200 OK
    const char* http_str = "HTTP/";
    *buf += strlen(http_str);
    float version_number = parse_http_version_number(buf);
    *buf += 1; // skip space

    int answer_code = parse_http_answer_code(buf);
    *buf += 1; // skip space

    char* answer_code_name = parse_http_answer_code_name(buf);
    
    *buf += 1; // pass '\n'

    // second line : Date: Sun, 09 Mar 2025 15:15:40 GMT

    const char* DATE_static_str = "DATE: ";
    *buf += strlen(DATE_static_str);

    char* date_str = parse_date_string(buf);

    *buf += 1; // pass '\n'

    // third line : Expires: -1
    // ignore this line

    ignore_line(buf);

    *buf += 1; // pass '\n'

    // fourth line : Cache-Control: private, max-age=0
    // ignore this line

    ignore_line(buf);

    *buf += 1; // pass '\n'

    return (struct http_header){
        .version_number = version_number,
        .answer_code = answer_code,
        .answer_code_name = answer_code_name,
        .date_str = date_str,
    };
}


void free_header(struct http_header header){
    free(header.answer_code_name);
    free(header.date_str);
}
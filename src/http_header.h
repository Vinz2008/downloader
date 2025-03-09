struct http_header {
    float version_number;
    int answer_code;
    char* answer_code_name;
    char* date_str;
};

struct http_header parse_header(char** buf);
void free_header(struct http_header header);
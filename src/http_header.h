// TODO : remove unused fields ?

enum Connection {
    CONNECTION_CLOSE,
    CONNECTION_OTHER,
};

enum TransferEncoding {
    TRANSFER_ENCODING_NONE,
    TRANSFER_ENCODING_CHUNKED,
    TRANSFER_ENCODING_OTHER,
};

struct http_header {
    float version_number;
    int answer_code;
    char* answer_code_name;
    char* date_str;
    char* content_type;
    char* content_type_options;
    char* server_name;
    char* location_redirection;
    enum Connection connection;
    enum TransferEncoding encoding;
};

struct http_header* parse_header(char** buf);
void free_header(struct http_header* header);
void debug_print_http_header(struct http_header* header);
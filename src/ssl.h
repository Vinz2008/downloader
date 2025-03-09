#include <openssl/ssl.h>

void init_ssl();
SSL_CTX* get_ssl_context();
void free_ssl_context_if_exists();
void log_ssl();
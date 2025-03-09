#include "ssl.h"
#include <openssl/err.h>

SSL_CTX* ssl_ctx = NULL;

void init_ssl(){
    SSL_load_error_strings();
    SSL_library_init();
    ssl_ctx = SSL_CTX_new(SSLv23_client_method());
}

SSL_CTX* get_ssl_context(){
    return ssl_ctx;
}

void log_ssl(){
    int err;
    while ((err = ERR_get_error())) {
        char *str = ERR_error_string(err, 0);
        if (!str)
            return;
        printf(str);
        printf("\n");
    }
}

void free_ssl_context_if_exists(){
    if (ssl_ctx){
        SSL_CTX_free(ssl_ctx);
    }
}

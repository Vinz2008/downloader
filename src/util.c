#include "util.h"
#include <stddef.h>
#include <string.h>

bool startswith(const char* s, const char* prefix){
    size_t len_prefix = strlen(prefix);
    size_t len_s_min = 0;
    while (s[len_s_min] != '\0' && len_s_min < len_prefix){
        len_s_min++;
    }
    if (len_prefix > len_s_min){
        return false;
    }
    int pos = 0;
    while (pos < len_prefix){
        if (s[pos] != prefix[pos]){
            return false;
        }
        pos++;
    }

    return true;
}
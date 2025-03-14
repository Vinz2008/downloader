#include "args.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Args parse_args(int argc, char** argv){
    const char* outfile = NULL;
    const char* url = NULL;
    
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-o") == 0){
            if (i >= argc){
                fprintf(stderr, "missing filename after -o \n");
                exit(1);
            }
            i++;
            outfile = argv[i];
        } else {
            url = argv[i];
        }
    }


    if (url == NULL){
        fprintf(stderr, "missing url\n");
        exit(1);
    }
    
    
    return (struct Args){
        .url = url,
        .outfile = outfile,
    };
}
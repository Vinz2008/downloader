#include <stdio.h>
#include "download.h"

int main(int argc, char** argv){
    if (argc-1 < 2){
        fprintf(stderr, "expected 2 args, got %d\n", argc-1);
        return 1;
    }
    if (!download(argv[1], argv[2])){
        printf("failed to download !\n");
        return 1;
    }
    printf("download succeeded\n");
    return 0;
}
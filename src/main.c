#include <stdio.h>
#include "download.h"
#include "args.h"

int main(int argc, char** argv){
    if (argc-1 < 1){
        fprintf(stderr, "No args were provided\n");
        return 1;
    }
    struct Args args = parse_args(argc, argv);
    if (!download(args.url, args.outfile)){
        printf("failed to download !\n");
        return 1;
    }
    printf("download succeeded\n");
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/fileutil.h"

void print_usage(char* argv) {
    printf("USAGE: %s <path_to_file>\n", argv);
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        print_usage(argv[0]);
        exit(1);
    }
    //get file contents
    char* path = argv[1];
    char* contents = get_file_contents(path);

    printf("Read: \n%s", contents);

    //lex

    //tokenize

    //parse


    return EXIT_SUCCESS;
}

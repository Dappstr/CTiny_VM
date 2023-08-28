#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int get_file_size(FILE *file) {
    if(!file) {
        printf("Failed to get file pointer in %s", __func__);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    unsigned int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(size < 1) {
        printf("Failed to get file size\n");
        exit(1);
    }
    else if(size == 1) {
        printf("File contains only EOF\n");
        exit(2);
    }

    return size;
}

char* get_file_contents(const char* path) {
    FILE* file = fopen(path, "r");
    if(!file) {
        printf("Failed to create file object in %s", __func__);
        exit(0);
    }
    
    unsigned int size = get_file_size(file);
    char* contents = malloc(size + 1);
    fread(contents, 1, size, file);
    contents[size] = '\0';

    return contents;
}

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
    //char** lexemes = lex(contents);

    //tokenize

    //parse


    return EXIT_SUCCESS;
}

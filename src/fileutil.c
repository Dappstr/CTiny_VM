#include <stdio.h>
#include <stdlib.h>

#include "../include/fileutil.h"

unsigned int get_file_size(FILE* file) {
    if(!file) {
        printf("Could not get file pointer in %s", __func__);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    unsigned int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(size < 1) {
        printf("Error getting file size!\n");
        exit(1);
    }
    
    else if(size == 1) {
        printf("File contains only EOF\n");
        exit(1);
    }

    return size;
}

char* get_file_contents(char* path) {
    FILE* file = fopen(path, "r");
    unsigned int size = get_file_size(file);

    char* contents = malloc(size + 1);
    fread(contents, 1, size, file);
    contents[size] = '\0';

    return contents;
}

#include <stdio.h>
#include <stdlib.h>

#include <alienos.h>


char *alien_init_loadfile(char *filename, long *file_size) {
    char *file_content = NULL;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fputs("init_loadfile: File descriptor is NULL.", stderr);
        exit(127);
    }

    if (fseek(fp, 0L, SEEK_END) != 0) {
        fputs("init_loadfile: Seeking to the end of file error.", stderr);
        exit(127);
    }

    *file_size = ftell(fp);
    if (*file_size == -1) {
        fputs("init_loadfile: Could not determine file size.", stderr);
        exit(127);
    }

    file_content = malloc(sizeof(char) * (*file_size + 1));

    if (fseek(fp, 0L, SEEK_SET) != 0) {
        fputs("init_loadfile: Could not seek to the file beggining.", stderr);
        exit(127);
    }

    size_t len = fread(file_content, sizeof(char), *file_size, fp);
    if (ferror(fp) != 0 ) {
        fputs("init_loadfile: Error reading file.", stderr);
        exit(127);
    }

    file_content[len++] = '\0';

    fclose(fp);

    return file_content;
}

int alien_init(int argc, char *argv[]) {
    long file_size;
    char *file = alien_init_loadfile(argv[1], &file_size);

    return 0;
}

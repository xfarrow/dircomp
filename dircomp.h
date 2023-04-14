/*

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

*/

# ifndef DIRCOMP_GUARD
# define DIRCOMP_GUARD

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <openssl/sha.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

struct arguments{
    char* directory1;
    char* directory2;
    bool r; // recursive
    bool v; // verbose
    bool h; // help
};

struct arguments get_arguments(int, char**);

// Reference: https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
bool analyze_directories(struct arguments*);

// Reference: https://www.openssl.org/docs/man1.1.1/man3/SHA512_Init.html
unsigned char* get_sha1_file(char *);

void print_help(void);

#endif
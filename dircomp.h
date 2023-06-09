/*
    dircomp - A directory comparison tool

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
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <sys/stat.h>

#define BYTES_TO_READ_AT_ONCE 512000    // 500KiB
#if BYTES_TO_READ_AT_ONCE > SIZE_MAX
#error Compile-time error: The specified value of BYTES_TO_READ_AT_ONCE is too large for this system.
#endif

struct arguments{
    char* directory1;
    char* directory2;
    bool r; // recursive
    bool v; // verbose
    bool h; // help
    bool f; // fast
    bool d; // comparison by digest
};

struct arguments get_arguments(int, char**);

// Reference: https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
bool analyze_directories(char*, char*, struct arguments*);

int byte_by_byte_file_comparison(char*, char*);

int hash_by_hash_file_comparison(char*, char*);

unsigned char* sha1(char*);

unsigned char* sha1_legacy(char*);

char* combine_path(char*, char*);

void print_help(void);

#endif

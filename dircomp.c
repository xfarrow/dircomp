/*

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <openssl/sha.h>
#include <string.h>
#include <limits.h>

struct arguments{
    char* directory1;
    char* directory2;
    bool r; // recursive
    bool n; // compare names only
    bool s; // compare hashes only
    bool v; // verbose (default, at the moment)
    bool h; // help
};

struct arguments get_arguments(int, char**);
void print_help(void);
void analyze_directories(char*, char*, struct arguments*);
unsigned char* get_sha1_file(char *);

int main(int argc, char* argv[]){
    struct arguments arguments = get_arguments(argc, argv);

    if(arguments.h == true){
        print_help();
        return 0;
    }

    analyze_directories(arguments.directory1, "", &arguments);

    return 0;
}

struct arguments get_arguments(int argc, char** argv){
    struct arguments provided_arguments = {"", "", false, false, false, true, false};
    char option;
    while((option = getopt(argc, argv, "rnsvh")) != -1){
        switch(option){
            case 'r':
                provided_arguments.r = true;
                break;
            case 'n':
                provided_arguments.n = true;
                break;
            case 's':
                provided_arguments.s = true;
                break;
            case 'v':
                provided_arguments.v = true;
                break;
            case 'h':
                provided_arguments.h = true;
                break;
        }
    }

    // Get directories
    if( (argc - optind) < 2 ){
         fprintf (stderr, "Not enough directories.\n");
         exit(-1);
    }
    else if( (argc - optind) > 2 ){
         fprintf (stderr, "Too many directories.\n");
         exit(-1);
    }
    provided_arguments.directory1 = malloc( (strlen(argv[optind]) * sizeof(char)) + 1);
    strcpy(provided_arguments.directory1, argv[optind]);
    provided_arguments.directory2 = malloc( (strlen(argv[optind + 1]) * sizeof(char)) + 1);
    strcpy(provided_arguments.directory2, argv[optind + 1]);
    
    return provided_arguments;
}

/*
*	References:
*	https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
*/
void analyze_directories(char* directory1, char* directory2, struct arguments* arguments){ // testing only
    printf("\nAnalyzing directory %s\n", directory1);
    DIR *d;
    struct dirent *dir;
    d = opendir(directory1);
    if (d) {
        while ((dir = readdir(d)) != NULL) {

            // is file
            if (dir -> d_type == DT_REG)
            {
                if(arguments -> v == true)
                    printf("Analyzing file: %s\n", dir -> d_name);
            }

            // is directory
            if (dir -> d_type == DT_DIR)
            {
                if(arguments -> r == true)
                    analyze_directories(dir -> d_name, "", arguments);
            }

        }
    closedir(d);
    }
}

/*
*   References: 
*   https://www.openssl.org/docs/man1.1.1/man3/SHA512_Init.html
*/
unsigned char* get_sha1_file(char *filename){
    FILE* f = fopen(filename,"rb");
    if(f == NULL){
        fprintf (stderr, "Couldn't open %s\n", filename);
        exit(-1);
    }

    // For a matter of efficiency, we do not read
    // the whole file at once. It'd be heavy on RAM.
    // Instead, we read BYTES_TO_READ_AT_ONCE at time

    #define BYTES_TO_READ_AT_ONCE 1048576 // 1MiB
    unsigned int bytes; // how many bytes we have actually read from fread
    #if BYTES_TO_READ_AT_ONCE > UINT_MAX
        #error Trying to read more bytes than what is possible to handle. Recompile using unsigned long or reduce BYTES_TO_READ_AT_ONCE
    #endif
    
    SHA_CTX context;
    unsigned char* hash = malloc(SHA_DIGEST_LENGTH * sizeof(unsigned char)); // result will be here
    unsigned char databuffer[BYTES_TO_READ_AT_ONCE];
    SHA1_Init(&context);
    while((bytes = fread(databuffer, 1, BYTES_TO_READ_AT_ONCE, f)) != 0){
        SHA1_Update(&context, databuffer, bytes);
    }
    SHA1_Final(hash, &context);
    fclose(f);
    return hash;
}

void print_help(void){
    printf("Usage: dircomp directory1 directory2 [-r] [-n] [-s] [-v] [-h]\n\n");
    printf("  -r \t\t Recursive\n");
    printf("  -n \t\t Compare file names only\n");
    printf("  -s \t\t Compare file hashes only\n");
    printf("  -v \t\t Verbose\n");
    printf("  -h \t\t Print this help and quit\n\n");
}

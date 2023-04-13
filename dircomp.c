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
void print_files_in_directory(char*, char*, struct arguments*);

int main(int argc, char* argv[]){
    struct arguments arguments = get_arguments(argc, argv);

    if(arguments.h == true){
        print_help();
        return 0;
    }

    print_files_in_directory(arguments.directory1, "", &arguments); // pass pointer to reduce memory usage

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
    if( (argc - optind) < 2){
         fprintf (stderr, "Not enough directories.\n");
         exit(-1);
    }
    else if( (argc - optind) > 2){
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
void print_files_in_directory(char* directory1, char* directory2, struct arguments* arguments){ // testing only
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
                    print_files_in_directory(dir -> d_name, "", arguments);
            }

        }
    closedir(d);
    }
}

void sha1(char* string){
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(string, strlen(string), hash);

    int i;
    for (i = 0; i < SHA_DIGEST_LENGTH; ++i)
	    printf("%02x", hash[i]);

    putchar('\n');
}

void print_help(void){
    printf("Usage: dircomp directory1 directory2 [-r] [-n] [-s] [-v] [-h]\n\n");
    printf("  -r \t\t Recursive\n");
    printf("  -n \t\t Compare file names only\n");
    printf("  -s \t\t Compare file hashes only\n");
    printf("  -v \t\t Verbose\n");
    printf("  -h \t\t Print this help and quit\n\n");
}

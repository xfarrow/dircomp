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
void print_files_in_directory(struct arguments);

int main(int argc, char* argv[]){
    struct arguments arguments = get_arguments(argc, argv);

    if(arguments.h == true){
        print_help();
        return 0;
    }

    arguments.directory1 = "/home/user";
    arguments.directory2 = "/home/user";
    print_files_in_directory(arguments);

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
    return provided_arguments;
}

void print_help(void){
    printf("Usage: dircomp directory1 directory2 [-r] [-n] [-s] [-v] [-h]\n\n");
    printf("  -r \t\t Recursive\n");
    printf("  -n \t\t Compare file names only\n");
    printf("  -s \t\t Compare file hashes only\n");
    printf("  -v \t\t Verbose\n");
    printf("  -h \t\t Print this help and quit\n\n");
}

/*
*	References:
*	https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
*/
void print_files_in_directory(struct arguments arguments){ // testing only
    printf("\nAnalyzing directory %s\n", arguments.directory1);
    DIR *d;
    struct dirent *dir;
    d = opendir(arguments.directory1);
    if (d) {
        while ((dir = readdir(d)) != NULL) {

            // is file
            if (dir -> d_type == DT_REG)
            {
                if(arguments.v == true)
                    printf("Analyzing file: %s\n", dir -> d_name);
            }

            // is directory
            if (dir -> d_type == DT_DIR)
            {
                if(arguments.r == true)
                    print_files_in_directory(dir -> d_name, arguments); // todo, pass the pointer to reduce memory usage
            }

        }
    closedir(d);
    }
}

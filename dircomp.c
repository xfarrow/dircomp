/*

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

*/

# include "dircomp.h"

int main(int argc, char* argv[]){
    struct arguments arguments = get_arguments(argc, argv);
    if(arguments.h == true){
        print_help();
        return 0;
    }
    if(analyze_directories(&arguments)){
        printf("Directories are equal\n");
    }
    else{
        printf("Directories are not equal\n");
    }
    return 0;
}

struct arguments get_arguments(int argc, char** argv){
    struct arguments provided_arguments = {"", "", false, false, false};
    char option;
    while((option = getopt(argc, argv, "rnsvh")) != -1){
        switch(option){
            case 'r':
                provided_arguments.r = true;
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

bool analyze_directories(struct arguments* arguments){
    if(arguments -> v == true)
        printf("\nAnalyzing directories %s %s\n", arguments -> directory1, arguments -> directory2);

    bool is_directory_equal = true;
    char fullpath[512];
    struct dirent* element;
    DIR *directory;

    // Open directory 1
    directory = opendir(arguments -> directory1);
    if (directory) {
        while ((element = readdir(directory)) != NULL) {

            // Is file
            if (element -> d_type == DT_REG)
            {
                // Check wether it exists in directory2
                strcpy(fullpath, arguments -> directory2);
                strcat(fullpath, "/");
                strcat(fullpath, element -> d_name);
                if( access(fullpath, F_OK ) == -1 )
                {
                    is_directory_equal = false;
                    if(arguments -> v == true)
                        printf("File %s exists in %s but does not in %s\n"  , element -> d_name 
                                                                            , arguments -> directory1
                                                                            , arguments -> directory2);
                }
                // Check if files have the same signature
                else{
                    strcpy(fullpath, arguments -> directory1);
                    strcat(fullpath, "/");
                    strcat(fullpath, element -> d_name);
                    unsigned char* hash_file_1 = get_sha1_file(fullpath);
                    strcpy(fullpath, arguments -> directory2);
                    strcat(fullpath, "/");
                    strcat(fullpath, element -> d_name);
                    unsigned char* hash_file_2 = get_sha1_file(fullpath);
                    if(strcmp(hash_file_1, hash_file_2) != 0){
                        is_directory_equal = false;
                        if(arguments -> v == true)
                            printf("File %s in %s has a different signature in %s\n"  , element->d_name
                                                                                    , arguments->directory1
                                                                                    , arguments->directory2);
                    }
                    free(hash_file_1);
                    free(hash_file_2);
                }
            }

            // Is directory
            else if (element -> d_type == DT_DIR)
            {
                // Check wether it exists in directory2
                strcpy(fullpath, arguments -> directory2);
                strcat(fullpath, "/");
                strcat(fullpath, element -> d_name);
                // Allocate heap memory in order to free it before a potential recursive call starts
                struct stat* dummy_structure = malloc(sizeof(struct stat));
                if(stat(fullpath, dummy_structure) == -1){
                    is_directory_equal = false;
                    if(arguments -> v == true)
                        printf("Sub-directory %s exists in %s but does not in %s\n"   , element -> d_name 
                                                                                    , arguments->directory1
                                                                                    , arguments->directory2);
                }
                free(dummy_structure);

                // Analyze recursively
                if(arguments -> r == true){
                    if(strcmp(element -> d_name, ".") == 0 || strcmp(element -> d_name, "..") == 0){
                        continue;
                    }
                    analyze_directories(arguments);
                }
            }
        }
        closedir(directory);
    }

    // Open directory 2
    directory = opendir(arguments -> directory2);
    if (directory) 
    {
        while ((element = readdir(directory)) != NULL) 
        {
            // Is file
            if (element -> d_type == DT_REG)
            {
                // Check wether it exists in directory1
                strcpy(fullpath, arguments -> directory1);
                strcat(fullpath, "/");
                strcat(fullpath, element -> d_name);
                if( access(fullpath, F_OK ) == -1 )
                {
                    is_directory_equal = false;
                    if(arguments -> v == true)
                        printf("File %s exists in %s but does not in %s\n", element -> d_name 
                                                                        , arguments -> directory2
                                                                        , arguments -> directory1);
                }
            }

            // Is directory
            else if (element -> d_type == DT_DIR)
            {
                // Check wether it exists in directory1
                strcpy(fullpath, arguments -> directory1);
                strcat(fullpath, "/");
                strcat(fullpath, element -> d_name);
                struct stat dummy_structure; // no need to be malloc-ed, as it'll be automatically free-d as the call ends
                if(stat(fullpath, &dummy_structure) == -1){
                    is_directory_equal = false;
                    if(arguments -> v == true)
                        printf("Sub-directory %s exists in %s but does not in %s\n"   , element->d_name 
                                                                                    , arguments->directory2
                                                                                    , arguments->directory1);
                }
            }
        }
        closedir(directory);
    }
    return is_directory_equal;
}

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
    printf("Usage: dircomp directory1 directory2 [-r] [-v] [-h]\n\n");
    printf("  -r \t\t Recursive\n");;
    printf("  -v \t\t Verbose\n");
    printf("  -h \t\t Print this help and quit\n\n");
}
/*

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

*/

#include "dircomp.h"

int main(int argc, char *argv[])
{
    struct arguments arguments = get_arguments(argc, argv);
    if (arguments.h == true)
    {
        print_help();
        return 0;
    }

    char* directory_to_analyze1 = malloc(strlen(arguments.directory1) * sizeof(char));
    char* directory_to_analyze2 = malloc(strlen(arguments.directory2) * sizeof(char));
    strcpy(directory_to_analyze1, arguments.directory1);
    strcpy(directory_to_analyze2, arguments.directory2);

    if (analyze_directories(directory_to_analyze1, directory_to_analyze2, &arguments))
    {
        printf("Directories are equal\n");
    }
    else
    {
        printf("Directories are not equal\n");
    }
    free(arguments.directory1);
    free(arguments.directory2);
    return 0;
}

struct arguments get_arguments(int argc, char **argv)
{
    struct arguments provided_arguments = {"", "", false, false, false};
    if(argc == 1){
        provided_arguments.h = true;
        return provided_arguments;
    }
    char option;
    while ((option = getopt(argc, argv, "rnsvh")) != -1)
    {
        switch (option)
        {
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
    if ((argc - optind) < 2)
    {
        fprintf(stderr, "Not enough directories.\n");
        exit(-1);
    }
    else if ((argc - optind) > 2)
    {
        fprintf(stderr, "Too many directories.\n");
        exit(-1);
    }
    provided_arguments.directory1 = malloc((strlen(argv[optind]) * sizeof(char)) + 1);
    strcpy(provided_arguments.directory1, argv[optind]);
    provided_arguments.directory2 = malloc((strlen(argv[optind + 1]) * sizeof(char)) + 1);
    strcpy(provided_arguments.directory2, argv[optind + 1]);
    return provided_arguments;
}

bool analyze_directories(char* directory_to_analyze_1, char* directory_to_analyze_2, struct arguments *arguments)
{
    if (arguments->v == true)
        printf("\nAnalyzing directories %s %s\n", directory_to_analyze_1, directory_to_analyze_2);

    bool is_directory_equal = true;
    int stat_result, file_equality_result;
    char* fullpath_file_helper;
    char* fullpath_file_helper2;
    char* subdirectory1;
    char* subdirectory2;
    struct dirent *element;
    DIR *directory;

    directory = opendir(directory_to_analyze_1);
    if (directory)
    {
        while ((element = readdir(directory)) != NULL)
        {
            // Is file
            if (element->d_type == DT_REG)
            {
                // Check wether it exists in directory2
                fullpath_file_helper = malloc(sizeof(char) * (strlen(directory_to_analyze_2) + strlen(element->d_name) + 2) );
                strcpy(fullpath_file_helper, directory_to_analyze_2);
                strcat(fullpath_file_helper, "/");
                strcat(fullpath_file_helper, element->d_name);
                if (access(fullpath_file_helper, F_OK) == -1)
                {
                    is_directory_equal = false;
                    if (arguments->v == true)
                        printf("File %s exists in %s but does not in %s\n"  , element->d_name
                                                                            , directory_to_analyze_1
                                                                            , directory_to_analyze_2);
                }

                // Check if the files are the same
                else
                {
                    fullpath_file_helper = malloc(sizeof(char) * (strlen(directory_to_analyze_1) + strlen(element->d_name) + 2) );
                    strcpy(fullpath_file_helper, directory_to_analyze_1);
                    strcat(fullpath_file_helper, "/");
                    strcat(fullpath_file_helper, element->d_name);
                    fullpath_file_helper2 = malloc(sizeof(char) * (strlen(directory_to_analyze_2) + strlen(element->d_name) + 2) );
                    strcpy(fullpath_file_helper2, directory_to_analyze_2);
                    strcat(fullpath_file_helper2, "/");
                    strcat(fullpath_file_helper2, element->d_name);
                    file_equality_result = are_files_equal(fullpath_file_helper, fullpath_file_helper2);
                    if (file_equality_result != 1)
                    {
                        is_directory_equal = false;
                        if(file_equality_result == 0 && arguments->v == true)
                        {
                            printf("File %s in %s is different in %s\n" , element->d_name
                                                                        , directory_to_analyze_1
                                                                        , directory_to_analyze_2);
                        }
                        else if(file_equality_result == -1)
                        {
                            printf("Error while comparing file %s in the directories %s, %s\n", element->d_name
                                                                                            , directory_to_analyze_1
                                                                                            , directory_to_analyze_2);
                        }
                    }
                }
                free(fullpath_file_helper);
                free(fullpath_file_helper2);
            }

            // Is directory
            else if (element->d_type == DT_DIR)
            {
                if (strcmp(element->d_name, ".") == 0 || strcmp(element->d_name, "..") == 0)
                {
                    continue;
                }
                // Check wether a folder with the same name exists in directory2
                fullpath_file_helper = malloc(sizeof(char) * (strlen(directory_to_analyze_2) + strlen(element->d_name) + 2) );
                strcpy(fullpath_file_helper, directory_to_analyze_2);
                strcat(fullpath_file_helper, "/");
                strcat(fullpath_file_helper, element->d_name);
                // Allocate heap memory in order to be able to free it before a potential recursive call starts
                struct stat *dummy_structure = malloc(sizeof(struct stat));
                stat_result = stat(fullpath_file_helper, dummy_structure);
                free(dummy_structure);
                free(fullpath_file_helper);
                if (stat_result == -1) // directory does not exist
                {
                    is_directory_equal = false;
                    if (arguments->v == true)
                        printf("Sub-directory %s exists in %s but does not in %s\n" , element->d_name
                                                                                    , directory_to_analyze_1
                                                                                    , directory_to_analyze_2);
                }
                // Analyze recursively
                else
                { 
                    if (arguments->r == true)
                    {

                        subdirectory1 = malloc(sizeof(char) * (strlen(directory_to_analyze_1) + strlen(element->d_name) + 2));
                        strcpy(subdirectory1, directory_to_analyze_1);
                        strcat(subdirectory1, "/");
                        strcat(subdirectory1, element->d_name);
                        subdirectory2 = malloc(sizeof(char) * (strlen(directory_to_analyze_2) + strlen(element->d_name) + 2));
                        strcpy(subdirectory2, directory_to_analyze_2);
                        strcat(subdirectory2, "/");
                        strcat(subdirectory2, element->d_name);
                        is_directory_equal = analyze_directories(subdirectory1, subdirectory2, arguments) && is_directory_equal;
                    }
                }
            }
        }
        closedir(directory);
    }

    directory = opendir(directory_to_analyze_2);
    if (directory)
    {
        while ((element = readdir(directory)) != NULL)
        {
            // Is file
            if (element->d_type == DT_REG)
            {
                // Check wether it exists in directory1
                fullpath_file_helper = malloc(sizeof(char) * (strlen(directory_to_analyze_1) + strlen(element->d_name) + 2) );
                strcpy(fullpath_file_helper, directory_to_analyze_1);
                strcat(fullpath_file_helper, "/");
                strcat(fullpath_file_helper, element->d_name);
                if (access(fullpath_file_helper, F_OK) == -1)
                {
                    is_directory_equal = false;
                    if (arguments->v == true)
                        printf("File %s exists in %s but does not in %s\n"  , element->d_name
                                                                            , directory_to_analyze_2
                                                                            , directory_to_analyze_1);
                }
                free(fullpath_file_helper);
            }
            
            // Is directory
            else if (element->d_type == DT_DIR)
            {
                // Check wether a folder with the same name exists in directory1
                fullpath_file_helper = malloc(sizeof(char) * (strlen(directory_to_analyze_1) + strlen(element->d_name) + 2) );
                strcpy(fullpath_file_helper, directory_to_analyze_1);
                strcat(fullpath_file_helper, "/");
                strcat(fullpath_file_helper, element->d_name);
                struct stat dummy_structure; // no need to be malloc-ed, as it'll be automatically free-d as the call ends
                if (stat(fullpath_file_helper, &dummy_structure) == -1)
                {
                    is_directory_equal = false;
                    if (arguments->v == true)
                        printf("Sub-directory %s exists in %s but does not in %s\n" , element->d_name
                                                                                    , directory_to_analyze_2
                                                                                    , directory_to_analyze_1);
                }
                free(fullpath_file_helper);
            }
        }
        closedir(directory);
    }
    free(directory_to_analyze_1);
    free(directory_to_analyze_2);
    return is_directory_equal;
}

int are_files_equal(char* filename1, char* filename2){

    if(strcmp(filename1, filename2) == 0)
        return 1; // it's the same path, so it's the same file

    struct stat stat1, stat2;

    if ( stat(filename1, &stat1) != 0 || stat(filename2, &stat2) != 0)
        return -1; // error opening files

    if(stat1.st_size != stat2.st_size)
        return 0; // files are not the same as they have a different dimension
    
    FILE *file1 = fopen(filename1, "rb");
    FILE *file2 = fopen(filename2, "rb");
    if (file1 == NULL || file2 == NULL)
    {
        return -1; // error opening files
    }
    #define BYTES_TO_READ_AT_ONCE 512000
    unsigned char databuffer1[BYTES_TO_READ_AT_ONCE] = "";
    unsigned char databuffer2[BYTES_TO_READ_AT_ONCE] = "";
    size_t bytes;
    while ((bytes = fread(databuffer1, 1, BYTES_TO_READ_AT_ONCE, file1)) != 0)
    {
        if(fread(databuffer2, 1, bytes, file2) != bytes){
            fclose(file1);
            fclose(file2);
            return -1; // error while reading the file(s)
        }
        if(memcmp(databuffer1, databuffer2, bytes) != 0){
            fclose(file1);
            fclose(file2);
            return 0; // files are not the same
        }
    }
    fclose(file1);
    fclose(file2);
    return 1;
}

void print_help(void)
{
    printf("usage: dircomp directory1 directory2 [-r] [-v] [-h]\n");
    printf("  -r \t\t Recursive\n");
    printf("  -v \t\t Verbose\n");
    printf("  -h \t\t Print this help and quit\n");
}
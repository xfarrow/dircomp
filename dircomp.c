/*
    dircomp - A directory comparison tool

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.

*/

#include "dircomp.h"

/// @brief Entry function
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
    free(arguments.directory1);
    free(arguments.directory2);
    arguments.directory1 = NULL;
    arguments.directory2 = NULL;

    if (analyze_directories(directory_to_analyze1, directory_to_analyze2, &arguments))
    {
        printf("Directories are equal\n");
    }
    else
    {
        printf("Directories are not equal\n");
    }

    return 0;
}

/// @brief Given the arguments passed to the program, construct a "struct arguments" representing its options
struct arguments get_arguments(int argc, char **argv)
{
    struct arguments provided_arguments = {"", "", false, false, false, false, false};
    if(argc == 1){
        provided_arguments.h = true;
        return provided_arguments;
    }
    char option;
    while ((option = getopt(argc, argv, "rvhfb")) != -1)
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
        case 'f':
            provided_arguments.f = true;
            break;
        case 'b':
            provided_arguments.b = true;
            break;
        }
    }

    // Get directories
    if ((argc - optind) < 2)
    {
        printf("Not enough directories.\n");
        exit(-1);
    }
    else if ((argc - optind) > 2)
    {
        printf("Too many directories.\n");
        exit(-1);
    }
    provided_arguments.directory1 = malloc((strlen(argv[optind]) * sizeof(char)) + 1);
    strcpy(provided_arguments.directory1, argv[optind]);
    provided_arguments.directory2 = malloc((strlen(argv[optind + 1]) * sizeof(char)) + 1);
    strcpy(provided_arguments.directory2, argv[optind + 1]);
    return provided_arguments;
}

/// @brief Checks if two directories are equivalent.
/// @param directory_to_analyze_1 Name of the first directory. Will be free-d before the call terminates
/// @param directory_to_analyze_2 Name of the second directory. Will be free-d before the call terminates
/// @param arguments Arguments passed to dircomp
/// @return true if the directories are equivalent, false otherwise
bool analyze_directories(char* directory_to_analyze_1, char* directory_to_analyze_2, struct arguments *arguments)
{
    bool is_directory_equal = true;
    int stat_result, file_equality_result;
    char *fullpath_helper, *fullpath_helper2;
    struct dirent *element;
    DIR *directory1, *directory2;

    if(strcmp(directory_to_analyze_1, directory_to_analyze_2) == 0){
        return true;
    }

    directory1 = opendir(directory_to_analyze_1);
    directory2 = opendir(directory_to_analyze_2);

    if(directory1 == NULL || directory2 == NULL)
    {
        printf("Couldn't open %s, %s, or both. The directories will be considered to be not equal.\n",
                directory_to_analyze_1, directory_to_analyze_2);
        free(directory_to_analyze_1);
        free(directory_to_analyze_2);
        return false;
    }

    while ((element = readdir(directory1)) != NULL)
    {
        // Is file
        if (element->d_type == DT_REG)
        {
            // Check whether it exists in directory2
            fullpath_helper = combine_path(directory_to_analyze_2, element->d_name);
            if (access(fullpath_helper, F_OK) == -1)
            {
                is_directory_equal = false;
                if (arguments->v)
                    printf("File %s exists in %s but does not in %s\n"  , element->d_name
                                                                        , directory_to_analyze_1
                                                                        , directory_to_analyze_2);
                free(fullpath_helper);
            }

            // If the file exists, check if they are the same
            else
            {
                fullpath_helper2 = combine_path(directory_to_analyze_1, element->d_name);

                if(arguments->b)
                    file_equality_result = byte_by_byte_file_comparison(fullpath_helper, fullpath_helper2);
                else
                    file_equality_result = hash_by_hash_file_comparison(fullpath_helper, fullpath_helper2);


                if (file_equality_result != 1)
                {
                    is_directory_equal = false;
                    if(file_equality_result == 0 && arguments->v)
                    {
                        printf("File %s in %s is different in %s\n" , element->d_name
                                                                    , directory_to_analyze_1
                                                                    , directory_to_analyze_2);
                    }
                    else if(file_equality_result == -1)
                    {
                        printf("An error occurred while comparing file %s in the directories %s, %s\n"  , element->d_name
                                                                                                        , directory_to_analyze_1
                                                                                                        , directory_to_analyze_2);
                    }
                }
                free( fullpath_helper );
                free( fullpath_helper2 );
            }
        }

        // Is directory
        else if (element->d_type == DT_DIR)
        {
            if (strcmp(element->d_name, ".") == 0 || strcmp(element->d_name, "..") == 0)
            {
                continue;
            }
            // Check whether a folder with the same name exists in directory2
            fullpath_helper = combine_path(directory_to_analyze_2, element->d_name);
            // Allocate heap memory in order to be able to free it before a potential recursive call starts
            struct stat *dummy_structure = malloc(sizeof(struct stat));
            stat_result = stat(fullpath_helper, dummy_structure);
            free(dummy_structure);
            free(fullpath_helper);
            if (stat_result == -1) // directory does not exist
            {
                is_directory_equal = false;
                if (arguments->v)
                    printf("Sub-directory %s exists in %s but does not in %s\n" , element->d_name
                                                                                , directory_to_analyze_1
                                                                                , directory_to_analyze_2);
            }
            else
            {
                // Analyze recursively
                if (arguments->r)
                {
                    fullpath_helper = combine_path(directory_to_analyze_1, element->d_name);
                    fullpath_helper2 = combine_path(directory_to_analyze_2, element->d_name);
                    is_directory_equal = analyze_directories(fullpath_helper, fullpath_helper2, arguments) && is_directory_equal;
                }
            }
        }
        if(arguments->f && !is_directory_equal){
            free(directory_to_analyze_1);
            free(directory_to_analyze_2);
            closedir(directory1);
            return false;
        }
    }
    closedir(directory1);

    while ((element = readdir(directory2)) != NULL)
    {
        if (element->d_type == DT_REG || element->d_type == DT_DIR)
        {
            fullpath_helper = combine_path(directory_to_analyze_1, element->d_name);
            if(element->d_type == DT_REG){
                if (access(fullpath_helper, F_OK) == -1)
                {
                    is_directory_equal = false;
                    if (arguments->v)
                    {
                        printf("File %s exists in %s but does not in %s\n"  , element->d_name
                                                                            , directory_to_analyze_2
                                                                            , directory_to_analyze_1);
                    }
                }
            }
            else if(element->d_type == DT_DIR)
            {
                struct stat dummy_structure; // no need to be malloc-ed, as it'll be automatically free-d as the call ends
                if (stat(fullpath_helper, &dummy_structure) == -1)
                {
                    is_directory_equal = false;
                    if (arguments->v)
                    {
                        printf("Sub-directory %s exists in %s but does not in %s\n" , element->d_name
                                                                                    , directory_to_analyze_2
                                                                                    , directory_to_analyze_1);
                    }
                }
            }
            free(fullpath_helper);
        }
        if(arguments->f && !is_directory_equal)
        {
            free(directory_to_analyze_1);
            free(directory_to_analyze_2);
            closedir(directory2);
            return false;
        }
    }
    closedir(directory2);

    free(directory_to_analyze_1);
    free(directory_to_analyze_2);

    return is_directory_equal;
}

/// @brief Checks if two files contain the same data (by comparing their content, one byte a time)
/// @param filename1 Name of the first file
/// @param filename2 Name of the second file
/// @return Returns 1 if the files are the same, 0 otherwise, -1 if an error occurred
int byte_by_byte_file_comparison(char* filename1, char* filename2)
{
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

/// @brief Checks if two files contain the same data (by comparing the resulting SHA-1 hash)
/// @param filename1
/// @param filename2
/// @return Returns 1 if the files are the same, 0 otherwise, -1 if an error occurred
int hash_by_hash_file_comparison(char* filename1, char* filename2)
{
    unsigned char* hash1 = sha1(filename1);
    unsigned char* hash2 = sha1(filename2);
    if(hash1 == NULL || hash2 == NULL)
    {
        return -1;
    }
    int ret = (memcmp(hash1, hash2, SHA_DIGEST_LENGTH) == 0);
    free(hash1);
    free(hash2);
    return ret;
}

/// @brief Generates the SHA-1 hash of a file. Deprecated since OpenSSL >= 3.0
/// @param filename Name of the file
/// @return Pointer to the digest
unsigned char* sha1_legacy(char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        fprintf(stderr, "Couldn't open %s\n", filename);
        return NULL;
    }

    // For a matter of efficiency, we do not read
    // the whole file at once. It'd be heavy on RAM.
    // Instead, we read BYTES_TO_READ_AT_ONCE at time
    size_t bytes; // how many bytes we have actually read from fread
    SHA_CTX context;
    unsigned char *hash = malloc(SHA_DIGEST_LENGTH * sizeof(unsigned char)); // result will be here
    unsigned char databuffer[BYTES_TO_READ_AT_ONCE];
    SHA1_Init(&context);
    while ((bytes = fread(databuffer, 1, BYTES_TO_READ_AT_ONCE, f)) != 0)
    {
        SHA1_Update(&context, databuffer, bytes);
    }
    SHA1_Final(hash, &context);
    fclose(f);
    return hash;
}

/// @brief Generates the SHA-1 hash of a file.
/// @param filename Name of the file
/// @return Pointer to the digest
unsigned char* sha1(char *filename)
{
    EVP_MD_CTX *mdctx; // envelope context
    const EVP_MD *md; // envelope mode (SHA1)
    unsigned char *hash = malloc(EVP_MAX_MD_SIZE * sizeof(unsigned char)); // result will be here
    unsigned int digest_len, i;
    size_t bytes; // how many bytes we have actually read from fread
    unsigned char databuffer[BYTES_TO_READ_AT_ONCE];

    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        fprintf(stderr, "Couldn't open %s\n", filename);
        return NULL;
    }

    mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        return NULL;
    }
    md = EVP_sha1();
    if (!EVP_DigestInit_ex(mdctx, md, NULL)) {
        return NULL;
    }

    while ((bytes = fread(databuffer, 1, BYTES_TO_READ_AT_ONCE, f)) != 0)
    {
        if (!EVP_DigestUpdate(mdctx, databuffer, bytes)) {
            return NULL;
        }
    }

    if (!EVP_DigestFinal_ex(mdctx, hash, &digest_len)) {
        return NULL;
    }
    EVP_MD_CTX_free(mdctx);
    fclose(f);
    return hash;
}

/// @brief Combines two paths
/// @param path1
/// @param path2
/// @return Pointer to the new path
char* combine_path(char* path1, char* path2){
    char* path = malloc(sizeof(char) * (strlen(path1) + strlen(path2) + 2) );
    strcpy(path, path1);
    strcat(path, "/");
    strcat(path, path2);
    return path;
}

/// @brief Output of the command 'dircomp -h'
/// @param
void print_help(void)
{
    printf("usage: dircomp directory1 directory2 [-rvfbh]\n");
    printf("  -r \t\t Recursive\n");
    printf("  -v \t\t Verbose\n");
    printf("  -f \t\t Fast. Halt as soon as the directories are found to be not equal\n");
    printf("  -b \t\t Byte-by-byte file comparison (default compares their hashes)\n");
    printf("  -h \t\t Print this help and quit\n");
}

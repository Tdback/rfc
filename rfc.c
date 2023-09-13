/*
 * rfc - an IETF RFC query tool for the terminal.
 */

#include <sys/stat.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    /* Contains sleep(u_int), getopt() function */
#include <curl/curl.h>

/* flags */
int f_clean;
int f_local;
int f_recent;
int f_save;
int f_format;

const char *version = "version 20230912";


char *setup_cache_path(void);
void open_and_write(const char *);


int main(int argc, char *argv[])
{
    int ch;

    if (argc == 1) {
        fprintf(stderr,
                "Usage: rfc [-V] [-c] [-l] [-r] [-f format] [-s] entry\n");
        return 1;
    }


    char *final_cache_dir = setup_cache_path();

    printf("Cache directory: %s\n", final_cache_dir);

    /* Create any necessary files. */
    struct stat sb;

    if (stat(final_cache_dir, &sb) == -1) {
        printf("Directory %s does not exist: creating is now...",
                final_cache_dir);

        if (0 != mkdir(final_cache_dir, 700)) {
            fprintf(stderr, "Error when trying to create directory %s\n",
                    final_cache_dir);
            exit(1);
        }

        printf("done; Directory %s successfully created\n", final_cache_dir);
    }

    /* Time to parse flags */
    const char *format = "txt";

    while ((ch = getopt(argc, argv, "Vclrsf:")) != -1) {
        switch (ch) {
        case 'V':
            fprintf(stderr, "rfc %s\n", version);
            return 1;
        case 'c':
            f_clean = 1;
            break;
        case 'l':
            f_local = 1;
            break;
        case 'r':
            f_recent = 1;
            break;
        case 's':
            f_save = 1;
            break;
        case 'f':
            if (strcmp(optarg, "txt") != 0 && strcmp(optarg, "html") != 0) {
                fprintf(stderr,
                        "Invalid format argument: '%s'\n"
                        "Expected value of 'txt' or 'html'...\n",
                        optarg);
                return 1;
            }

            f_format = 1;
            format = optarg;
            break;
        default:
            fprintf(stderr, "Unknown option %s is ignored.\n", optarg);
            return 1;
        }
    }
    argc -= optind;
    argv += optind;

    /* If -c option, clean out local cache. */
    if (f_clean && (f_local || f_recent || f_save || f_format || argc != 0)) {
        fprintf(stderr, "Incorrect usage of option -c\n");
        return 1;
    }

    if (f_clean && !f_local && !f_recent && !f_save && !f_format && argc == 0)
        printf("Cleaning shiz up!\n");

    /* If -l option, return list of all locally cached files. */
    if (f_local && (f_clean || f_recent || f_save || f_format || argc != 0)) {
        fprintf(stderr, "Incorrect usage of option -l\n");
        return 1;
    }

    if (f_local && !f_clean && !f_recent && !f_save && !f_format && argc == 0) {
        printf("Viewing local files\n");
    }

    /* If -r option, return list of all recently queried entries. */
    if (f_recent && (f_local || f_clean || f_save || f_format || argc != 0)) {
        fprintf(stderr, "Incorrect usage of option -r\n");
        return 1;
    }

    if (f_recent && !f_local && !f_clean && !f_save && !f_format && argc == 0) {
        /*
         * These files should be stored in a simple txt file within the RFC_PATH
         * directory, and it should only maintain a list of the last 5-10 files.
         */
        printf("Viewing recently queried files!\n");
        return 0;
    }

    /*
     * At this point check if there are any additional arguments passed.
     * If that's the case, exit out of the program.
     */
    if (argc > 1) {
        fprintf(stderr, "Too many arguments passed.\n");
        return 1;
    }

    /* Time to do some fun string concatenation in C :) */
    char *entry_to_curl = calloc(strlen(*argv) + strlen(format) + 2,
                                 sizeof(char));
    if (entry_to_curl == NULL) {
        fprintf(stderr, "calloc failed\n");
        return 1;
    }

    /* Format the entry string */
    strcat(entry_to_curl, *argv);
    strcat(entry_to_curl, ".");
    strcat(entry_to_curl, format);

    printf("%s\n", entry_to_curl);
    // curl_entry(entry_to_curl);

    /* If -s option, save the entry name in the specified format */
    if (f_save && !f_local && !f_clean && !f_recent) {
        printf("Saving entry %s\n", entry_to_curl);
        // save_entry_local(*argv);
    }

    /* Free all the calloc'd strings */
    free(final_cache_dir);
    free(entry_to_curl);

    return 0;
}

char *setup_cache_path()
{

    /* Create a cache directory in the user's home directory */
    const char *home_dir = getenv("HOME");
    if (home_dir == NULL) {
        perror("getenv");
        exit(1);
    }

    const char *cache_dir = getenv("RFC_PATH");
    if (cache_dir == NULL) {
        fprintf(stderr, "RFC_PATH is not set! Defaulting to \"%s/.cache/\".\n",
                home_dir);
        cache_dir = ".cache/";
    }

    /* Need to add 2 here for both the null byte and the "/" */
    char *final_cache_dir = calloc(strlen(home_dir) + strlen(cache_dir) + 2,
                                   sizeof(char));
    if (final_cache_dir == NULL) {
        fprintf(stderr, "calloc failed\n");
        exit(1);
    }

    /* Format the string holding the cache directory location */
    strcat(final_cache_dir, home_dir);
    strcat(final_cache_dir, "/");
    strcat(final_cache_dir, cache_dir);

    return final_cache_dir;
}

/* Call this if using curl to save the file locally */
void write_callback(void *contents, size_t size, size_t nitems, FILE *file)
{
    // printf("%.*s", (int)(size * nitems), (char *)contents);
    fwrite(contents, size, nitems, file);
}


/*
 * Will have to write using fopen contents of curl request to filenames,
 * most likely in format rfcNUMBER.{txt,html}
 */

/*
 * Use the following for getting the environment varialble RFC_PATH,
 * which specifies the path for locally cached RFC entries.
 *
 *      char * getenv (const char *name)
 *      char * secure_getenv (const char *name)
 *
 * Need to find a way to set a "default" when the environment var is not set,
 * or just set the environment variable ourselves :)
 * (first solution is probably the smartest move)
 */

/*
 * CURL *curl = curl_easy_init();
 * if (curl) {
 *     // const char *filename = "test.txt";
 *     // FILE *output_file = fopen(filename, "w");
 *     // if (!output_file) {
 *     //     fprintf(stderr, "Failed to open output file %s\n", filename);
 *     //     return 1;
 *     // }
 *     curl_easy_setopt(curl, CURLOPT_URL, "https://www.rfc-editor.org/rfc/rfc791.txt");
 *     // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
 *     curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
 *     curl_easy_perform(curl);
 *     curl_easy_cleanup(curl);
 *     // fclose(output_file);

 * }
 */


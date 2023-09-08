/*
 * Header comment and copyright license.
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
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

const char *version = "version 20230907";

int
main(int argc, char *argv[])
{
    /*
     * Rewrite these with getopt and check flags f_clean, f_save, etc.
     * Get rid of --help and simplify --version. Instead of help, just
     * print out the usage. Otherwise users should read the man page.
     */

    int ch;

    if (argc == 1) {
        fprintf(stderr,
                "Usage: rfc [-V] [-c] [-l] [-r [n]] [-f format] [-s] entry\n");
        return 1;
    }



    /*
    printf("Cleaning up locally cached entries...");
    Run clean function
    sleep(1);
    printf(" done; Successfully cleaned cache.\n");
    */

    const char *format = "txt";
    int view_recent = 5;
    /* Initialize flags */
    // char *ep;
    int errno;
    // long lval;

    while ((ch = getopt(argc, argv, "Vclr:sf:")) != -1) {
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
            /*
             * Fix this, should call a function to verify optarg is a valid
             * number. Otherwise it'll accept anything as input.
             */
            view_recent = atoi(optarg);
            break;
        case 's':
            f_save = 1;
            break;
        case 'f':
            f_format = 1;
            if (strcmp(optarg, "txt") == 0 || strcmp(optarg, "html") == 0) {
                format = optarg;
            } else {
                fprintf(stderr,
                        "Invalid format argument: "
                        "Expected value of 'txt' or 'html'...\n");
                return 1;
            }
            break;
        default:
            fprintf(stderr, "Unknown option %s is ignored.\n", optarg);
            return 1;
        }
    }
    argc -= optind;
    argv += optind;

    /*
     * If -c option, clean out cache. However, make sure no other options
     * are called with it, or print an error message.
     */
    if (f_clean && (f_local || f_recent || f_save || f_format || argc != 0)) {
        fprintf(stderr, "Incorrect usage of option -c\n");
        return 1;
    }

    if (f_clean && !f_local && !f_recent && !f_save && !f_format && argc == 0)
        printf("Cleaning shiz up!\n");

    /*
     * If -l option, return cached files. Similar to -c no other options should
     * be called with it or print an error message.
     */
    if (f_local && (f_clean || f_recent || f_save || f_format || argc != 0)) {
        fprintf(stderr, "Incorrect usage of option -l\n");
        return 1;
    }

    if (f_local && !f_clean && !f_recent && !f_save && !f_format && argc == 0)
        printf("Viewing local files\n");

    /*
     * If -r option, return [n] most recent searches. Print an error if called
     * with any other options.
     */
    if (f_recent && (f_local || f_clean || f_save || f_format || argc != 0)) {
        fprintf(stderr, "Incorrect usage of option -r\n");
        return 1;
    }

    if (f_recent && !f_local && !f_clean && !f_save && !f_format && argc == 0)
        printf("Viewing recent %d files!\n", view_recent);

    /* Handle the entry name */
    if (f_save && argc != 0)
        for (; *argv != NULL; argv++) {
            printf("%s\n", *argv);
        }

    /* Need to handle saving files */
    // printf("Saving file %s!\n", "hello");


    printf("Format chosen: %s\n", format);

    /*
    CURL *curl = curl_easy_init();
    if (curl) {
        // const char *filename = "test.txt";
        // FILE *output_file = fopen(filename, "w");
        // if (!output_file) {
        //     fprintf(stderr, "Failed to open output file %s\n", filename);
        //     return 1;
        // }
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.rfc-editor.org/rfc/rfc791.txt");
        // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        // fclose(output_file);

    }
    */

    return 0;
}

/* Call this if using curl to save the file locally */
void
write_callback(void *contents, size_t size, size_t nitems, FILE *file)
{
    // printf("%.*s", (int)(size * nitems), (char *)contents);
    fwrite(contents, size, nitems, file);
}

/*
 * Create function to validate that the input is a number
 * Need to understand this better

 * errno = 0;
 * lval = strtol(optarg, &ep, 10);
 * if (optarg[0] == '\0' || *ep != '\0') {
 *     fprintf(stderr, "Not a number!\n");
 *     return 1;
 * }
 * if ((errno == ERANGE && (lval == LONG_MAX || lval == LONG_MIN)) ||
 *         (lval > INT_MAX || lval < INT_MIN)) {
 *     fprintf(stderr, "Out of range!\n");
 *     return 1;
 * }
 * view_recent = lval;
 */

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

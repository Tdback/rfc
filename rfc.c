/*
 *
 * Copyright © 2023 Tyler Dunneback <tylerdback@pm.me>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>    /* For optarg int validation */
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

void open_and_write(const char *);


int main(int argc, char *argv[])
{
    int ch;

    if (argc == 1) {
        fprintf(stderr,
                "Usage: rfc [-V] [-c] [-l] [-r [n]] [-f format] [-s] entry\n");
        return 1;
    }


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

    if (f_local && !f_clean && !f_recent && !f_save && !f_format && argc == 0) {
        printf("Viewing local files\n");
    }

    /*
     * If -r option, return [n] most recent searches. Print an error if called
     * with any other options.
     */
    if (f_recent && (f_local || f_clean || f_save || f_format || argc != 0)) {
        fprintf(stderr, "Incorrect usage of option -r\n");
        return 1;
    }

    if (f_recent && !f_local && !f_clean && !f_save && !f_format && argc == 0) {
        printf("Viewing recent %d files!\n", view_recent);
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

    free(entry_to_curl);

    return 0;
}


/* Call this if using curl to save the file locally */
void write_callback(void *contents, size_t size, size_t nitems, FILE *file)
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


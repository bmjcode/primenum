/*
 * A naive prime number sieve.
 * Copyright (c) 2022 Benjamin Johnson <bmjcode@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "primenum.h"

/* Start a new log */
/* Set path to NULL to print output to the screen only. */
static FILE *log_start(struct primenum_list *list,
                       const char *path);

/* Write a value to the log and display it on screen */
static int log_write(primenum_int value, void *log);

/* Close the log */
static void log_close(FILE *log);

/* Display usage instructions */
static void usage(FILE *stream, char *exe_path);


FILE *
log_start(struct primenum_list *list, const char *path)
{
    FILE *log;
    struct primenum_entry *curr;

    if (path == NULL)
        log = NULL;
    else
        log = fopen(path, "wb");

    if (list != NULL) {
        /* Log existing entries in the list */
        for (curr = list->head;
             curr != NULL;
             curr = curr->next) {
            if (log_write(curr->value, log) != PRIMENUM_OK) {
                log_close(log);
                log = NULL;
                break;
            }
        }
    }
    return log;
}

int
log_write(primenum_int value, void *log)
{
    bool status;

    if (log == NULL)
        status = PRIMENUM_OK; /* so we can print it on screen */
    else if (fwrite(&value, sizeof(primenum_int), 1, log) == 1)
        status = PRIMENUM_OK; /* successfully written  to disk */
    else
        status = PRIMENUM_DISK_FULL;

    if (status == PRIMENUM_OK)
        printf("%"PRIMENUM_FMT"\n", value);
    return status;
}

void
log_close(FILE *log)
{
    if (log != NULL)
        fclose(log);
}

void
usage(FILE *stream, char *exe_path)
{
    fprintf(stream,
            "Usage: %s [-h] [-d PATH] [-l PATH] [-m MAX] [-n NUM]\n"
            "  -h       Display this help message and exit\n"
            "  -d PATH  Dump found primes to the specified file (implies -l)\n"
            "  -l PATH  Load previously found primes from the specified file\n"
            "  -m MAX   Stop after reaching the specified maximum value\n"
            "  -n NUM   Stop after finding the specified number of primes\n",
            exe_path);
}

int
main(int argc, char **argv)
{
    int opt, status;
    struct primenum_list *list;
    primenum_stop_cb stop_cb;
    primenum_int upper_bound;
    const char *log_path;
    FILE *log;

    list = primenum_list_new(true);
    stop_cb = primenum_stop_never; /* unless overridden */
    upper_bound = 0;
    log_path = NULL;

    while ((opt = getopt(argc, argv, "hd:l:m:n:")) != -1) {
        switch (opt) {
            case 'd':
                log_path = optarg;
                /* falls through to case 'l' */
            case 'l':
                primenum_load_from_disk(list, optarg);
                break;
            case 'm':
                stop_cb = primenum_stop_at_value;
                upper_bound = atol(optarg);
                printf("upper_bound = %"PRIMENUM_FMT"\n", upper_bound);
                break;
            case 'n':
                stop_cb = primenum_stop_at_count;
                upper_bound = atol(optarg);
                break;
            case 'h': /* display help nicely */
                usage(stdout, argv[0]);
                return 0;
            default: /* '?'; display help passive-aggressively */
                usage(stderr, argv[0]);
                return 1;
        }
    }

    if (optind < argc) {
        /* don't accept gratuitous command-line arguments */
        usage(stderr, argv[0]);
        return 1;
    }

    log = log_start(list, log_path);
    if ((log_path != NULL) && (log == NULL))
        status = PRIMENUM_DISK_FULL; /* already? */
    else
        status = primenum_test_loop(list,
                                    stop_cb, upper_bound,
                                    log_write, log);

    /* If an error occurred, indicate what happened */
    switch (status) {
        case PRIMENUM_OVERFLOW:
            fprintf(stderr, "Maximum value reached\n");
            status = PRIMENUM_OK; /* we've successfully done all we can */
            break;
        case PRIMENUM_MEM_FULL:
            fprintf(stderr, "Out of memory\n");
            break;
        case PRIMENUM_DISK_FULL:
            fprintf(stderr, "Out of disk space\n");
            break;
        case PRIMENUM_INVALID:
            fprintf(stderr, "Invalid data encountered\n");
            break;
    }

    log_close(log);
    primenum_list_free(list);
    return status;
}

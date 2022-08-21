/*
 * A naive prime factorization tool.
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

/* Display usage instructions */
static void usage(FILE *stream, char *exe_path);


void
usage(FILE *stream, char *exe_path)
{
    fprintf(stream,
            "Usage: %s [-h] [-l PATH] VALUE [VALUE ...]\n"
            "  -h       Display this help message and exit\n"
            "  -l PATH  Load known primes from the specified file\n",
            exe_path);
}

int
main(int argc, char **argv)
{
    int arg, opt;
    struct primenum_list *list, *factors;
    primenum_int value;
    struct primenum_entry *factor;

    list = primenum_list_new(true);

    while ((opt = getopt(argc, argv, "hl:")) != -1) {
        switch (opt) {
            case 'l':
                primenum_load_from_disk(list, optarg);
                break;
            case 'h': /* display help nicely */
                usage(stdout, argv[0]);
                return 0;
            default: /* '?'; display help passive-aggressively */
                usage(stderr, argv[0]);
                return 1;
        }
    }

    /* We need at least one command-line argument */
    if (optind >= argc) {
        usage(stderr, argv[0]);
        return 1;
    }

    /* Factor values passed on the command line */
    for (arg = optind; arg < argc; ++arg) {
        value = atol(argv[arg]);
        factors = primenum_factors(list, value, NULL, NULL);
        if (factors == NULL) {
            fprintf(stderr, "Out of memory\n"); /* well, probably */
            break;
        }

        printf("%"PRIMENUM_FMT":", value);
        factor = factors->head;
        while (factor != NULL) {
            printf(" %"PRIMENUM_FMT, factor->value);
            factor = factor->next;
        }
        printf("\n");
        primenum_list_free(factors);
    }

    primenum_list_free(list);
    return 0;
}

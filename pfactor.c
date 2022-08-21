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
            "Usage: %s [-h] [-e] [-l PATH] VALUE [VALUE ...]\n"
            "  -h       Display this help message and exit\n"
            "  -e       Display repeated factors using exponential notation\n"
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
    bool use_exponents;

    list = primenum_list_new(true);
    use_exponents = false;

    while ((opt = getopt(argc, argv, "hel:")) != -1) {
        switch (opt) {
            case 'e':
                use_exponents = true;
                break;
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
        if (use_exponents) {
            primenum_int last_base;
            unsigned int exponent;

#define PRINT_EXPONENT(last_base, exponent) \
        (exponent == 1) \
        ? printf(" %"PRIMENUM_FMT, last_base) \
        : printf(" %"PRIMENUM_FMT"^%u", last_base, exponent)
            last_base = factors->head->value;
            exponent = 0; /* the first time through the for-loop adds 1 */
            for (factor = factors->head;
                 factor != NULL;
                 factor = factor->next) {
                if (factor->value == last_base)
                    exponent++;
                else {
                    PRINT_EXPONENT(last_base, exponent);
                    last_base = factor->value;
                    exponent = 1;
                }
            }
            /* The for-loop stops before printing the last factor */
            PRINT_EXPONENT(last_base, exponent);
#undef PRINT_EXPONENT
        } else {
            for (factor = factors->head;
                 factor != NULL;
                 factor = factor->next)
                printf(" %"PRIMENUM_FMT, factor->value);
        }
        printf("\n");
        primenum_list_free(factors);
    }

    primenum_list_free(list);
    return 0;
}

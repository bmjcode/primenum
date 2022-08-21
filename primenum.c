/*
 * A naive prime number sieve and prime factorization library.
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
#include <tgmath.h>

#include "primenum.h"

bool
primenum_stop_never(primenum_int upper_bound,
                    struct primenum_list *list, primenum_int candidate)
{
    (void)upper_bound;
    (void)list;
    (void)candidate;
    return false;
}

bool
primenum_stop_at_value(primenum_int upper_bound,
                       struct primenum_list *list, primenum_int candidate)
{
    (void)list;
    return (candidate > upper_bound);
}

bool
primenum_stop_at_count(primenum_int upper_bound,
                       struct primenum_list *list, primenum_int candidate)
{
    (void)candidate;
    return (list->size >= upper_bound);
}

bool
primenum_test_inner(struct primenum_list *list, primenum_int value)
{
    bool prime;
    primenum_int root;
    struct primenum_entry *factor;

    /* Assume this value is prime until proven otherwise */
    prime = true;

    /* We can stop testing at the square root because if value == a * b,
     * finding a <= sqrt(value) implies the existence of b >= sqrt(value) */
    root = floor(sqrt(value));

    /* This value is composite if it is divisible by any of the (smaller)
     * primes we've already found */
    factor = list->head;
    while ((prime)
           && (factor != NULL)
           && (factor->value <= root)) {
        if (value % factor->value == 0)
            prime = false;
        factor = factor->next;
    }
    return prime;
}

int
primenum_test(struct primenum_list *list, primenum_int value,
              primenum_found_cb found_cb, void *cb_data)
{
    int status;

    status = PRIMENUM_OK; /* until proven otherwise */
    if (value < list->tail->value)
        status = PRIMENUM_OVERFLOW; /* we've tested the largest value we can */
    else if (primenum_test_inner(list, value)) {
        if (primenum_list_add(list, value) == NULL)
            status = PRIMENUM_MEM_FULL; /* we've run out of memory */
        else if (found_cb != NULL)
            status = found_cb(value, cb_data);
    }
    return status;
}

int
primenum_test_loop(struct primenum_list *list,
                   primenum_stop_cb stop_cb,
                   primenum_int upper_bound,
                   primenum_found_cb found_cb,
                   void *cb_data)
{
    int status;
    primenum_int candidate;

    status = PRIMENUM_OK; /* until proven otherwise */
    /* Sanity check */
    if (stop_cb == NULL)
        stop_cb = primenum_stop_never;

    /* We know two categories of numbers are always composite: Even numbers
     * greater than 2, and multi-digit numbers whose last digit is 5. In
     * other words, we know the last digit of any multi-digit prime is either
     * 1, 3, 7, or 9. This considerably helps to narrow our search. */
    candidate = list->tail->value + 2;
    /* Watch for obviously invalid candidates */
    if ((candidate > 2) && (candidate % 2 == 0))
        return PRIMENUM_INVALID;
    /* Skip multiples of five */
    else if ((candidate > 5) && (candidate % 5 == 0))
        candidate += 2;

#define WHILE_COND(cand) \
        ((status == PRIMENUM_OK) && (!stop_cb(upper_bound, list, (cand))))
    while (WHILE_COND(candidate)) {
        status = primenum_test(list, candidate, found_cb, cb_data);
        /* A little loop unrolling lets us skip multiples of 5 */
        if (candidate % 10 == 1) {
            if (!WHILE_COND(candidate + 2))
                break;
            status = primenum_test(list, candidate + 2, found_cb, cb_data);
            if (!WHILE_COND(candidate + 6))
                break;
            status = primenum_test(list, candidate + 6, found_cb, cb_data);
            if (!WHILE_COND(candidate + 8))
                break;
            status = primenum_test(list, candidate + 8, found_cb, cb_data);
            candidate += 10;
        } else
            candidate += 2;
    }
#undef WHILE_COND

    return status;
}

struct primenum_list *
primenum_factors(struct primenum_list *list, primenum_int value,
                 primenum_factor_cb factor_cb, void *cb_data)
{
    int status;
    struct primenum_list *factors;
    struct primenum_entry *candidate;

    /* Enumerate potential factors */
    /* Note that unlike in primenum_test_inner(), here we do have to
     * test values on both sides of the square root */
    status = primenum_test_loop(list,
                                primenum_stop_at_value, value,
                                NULL, NULL);
    if (status != PRIMENUM_OK)
        return NULL;

    factors = primenum_list_new(false);
    if (factors == NULL)
        return NULL; /* what just happened? */

    candidate = list->head;
    while ((candidate != NULL)
           && (candidate->value <= value)) {
        while (value % candidate->value == 0) {
            if (primenum_list_add(factors, candidate->value) == NULL) {
                primenum_list_free(factors);
                return NULL; /* what just happened? */
            }
            if (factor_cb != NULL)
                factor_cb(value, cb_data);
            value /= candidate->value;
        }
        candidate = candidate->next;
    }
    return factors;
}

void
primenum_load_from_disk(struct primenum_list *list, const char *path)
{
    FILE *log;
    primenum_int value;

    if ((list != NULL) && (path != NULL)) {
        log = fopen(path, "rb");
        if (log != NULL) {
            while (fread(&value, sizeof(primenum_int), 1, log) == 1) {
                /* Don't add values smaller than the last one in the list.
                 * This is mainly to avoid repeating the single digit primes
                 * added by primenum_list_new(). */
                if ((list->tail != NULL) && (value > list->tail->value)) {
                    if (primenum_list_add(list, value) == NULL)
                        break;
                }
            }
        }
        fclose(log);
    }
}

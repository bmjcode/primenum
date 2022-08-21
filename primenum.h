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

#ifndef PRIMENUM_H
#define PRIMENUM_H

#ifdef __cplusplus
extern "C" {
#endif


#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>


/*
 * Type definitions
 */

typedef uint64_t primenum_int;  /* numeric type for all operations */
#define PRIMENUM_FMT PRIu64     /* printf() format specifier for the above */

struct primenum_list;
struct primenum_entry;

/* A linked list of found primes, with fast append */
struct primenum_list {
    struct primenum_entry *head;
    struct primenum_entry *tail;
    primenum_int size;
};

/* An individual item in the above list */
struct primenum_entry {
    primenum_int value;
    struct primenum_entry *next;
};

/* Status codes for prime_test() and its ilk */
enum {
    PRIMENUM_OK,        /* success */
    PRIMENUM_OVERFLOW,  /* we've reached the maximum value of primenum_int */
    PRIMENUM_MEM_FULL,  /* we're out of memory */
    PRIMENUM_DISK_FULL, /* we're out of disk space */
    PRIMENUM_INVALID    /* we've encountered invalid data */
};

/* Signature for a callback function called when a prime number is found.
 * This function should return one of the status codes enumerated above. */
typedef int (*primenum_found_cb)(primenum_int value, void *data);

/* Signature for a callback function when a prime factor is found. */
typedef void (*primenum_factor_cb)(primenum_int value, void *data);

/* Signature for a callback function to set a stop condition for testing.
 * This function should return true when the condition is reached. */
typedef bool (*primenum_stop_cb)(primenum_int upper_bound,
                                 struct primenum_list *list,
                                 primenum_int candidate);


/*
 * Stop condition functions for use with primenum_test_loop()
 */

/* Keep going until we overflow or run out of memory */
bool primenum_stop_never(primenum_int upper_bound,
                         struct primenum_list *list,
                         primenum_int candidate);

/* Stop testing at a specified maximum value */
bool primenum_stop_at_value(primenum_int upper_bound,
                            struct primenum_list *list,
                            primenum_int candidate);

/* Stop testing when a specified number of primes are found */
bool primenum_stop_at_count(primenum_int upper_bound,
                            struct primenum_list *list,
                            primenum_int candidate);


/*
 * Function definitions
 */

/* Start a new list */
/* If populate == true, the list will be populated with the single-digit
 * primes. This is necessary for our algorithm, which works by trial division
 * against previously identified primes. */
struct primenum_list *primenum_list_new(bool populate);

/* Add a value to the list of found primes */
/* This returns the entry for the added value. Do with it what you will. */
struct primenum_entry *primenum_list_add(struct primenum_list *list,
                                         primenum_int value);

/* Delete the list of found primes */
void primenum_list_free(struct primenum_list *list);

/* Return whether a given value is prime */
bool primenum_test_inner(struct primenum_list *list,
                         primenum_int value);

/* Run the above test, and if the value is prime, add it to our list */
/* This returns one of the status codes enumerated above. */
int primenum_test(struct primenum_list *list,
                  primenum_int value,
                  primenum_found_cb found_cb,
                  void *cb_data);

/* Run the above test in a loop until we reach a specified stop condition */
/* This returns one of the status codes enumerated above. */
int primenum_test_loop(struct primenum_list *list,
                       primenum_stop_cb stop_cb,
                       primenum_int upper_bound,
                       primenum_found_cb found_cb,
                       void *cb_data);

/* Return a list containing the prime factors of the specified value */
struct primenum_list *primenum_factors(struct primenum_list *list,
                                       primenum_int value,
                                       primenum_factor_cb factor_cb,
                                       void *cb_data);

/* Load previously found primes from disk */
/* The file format is an ordered sequence of raw primenum_int values.
 * This can be a convenient time saver, but beware there is no guarantee
 * the data is valid, since testing it would be equivalent to regenerating
 * it from scratch. Use this at your own peril. */
void primenum_load_from_disk(struct primenum_list *list,
                             const char *path);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PRIMENUM_H */

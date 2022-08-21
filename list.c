/*
 * A linked list of integers with fast append.
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

#include <stdlib.h>

#include "primenum.h"

struct primenum_list *
primenum_list_new(bool populate)
{
    struct primenum_list *list;

    list = malloc(sizeof(struct primenum_list));
    if (list != NULL) {
        list->head = NULL;
        list->tail = NULL;
        list->size = 0;

        if (populate) {
            /* Seed the list with the single-digit primes */
            primenum_list_add(list, 2);
            primenum_list_add(list, 3);
            primenum_list_add(list, 5);
            primenum_list_add(list, 7);
        }
    }
    return list;
}

struct primenum_entry *
primenum_list_add(struct primenum_list *list, primenum_int value)
{
    struct primenum_entry *entry;

    entry = malloc(sizeof(struct primenum_entry));
    if (entry != NULL) {
        entry->value = value;
        entry->next = NULL;

        /* Start the list if it's empty */
        if (list->head == NULL)
            list->head = entry;

        /* Add this after the current last entry */
        if (list->tail != NULL)
            list->tail->next = entry;

        /* Make this the new last entry */
        list->tail = entry;

        /* Increment the list size */
        list->size++;
    }
    return entry;
}

void
primenum_list_free(struct primenum_list *list)
{
    struct primenum_entry *curr, *next;

    curr = list->head;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
    free(list);
}

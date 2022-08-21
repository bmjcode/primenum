If you're a serious math person interested in discovering very large prime numbers, this is not for you. Go check out [Prime95](https://www.mersenne.org/download/) for that. These are a couple stupid little programs for the kind of people who get curious how to factor their phone numbers.

[primenum.h](primenum.h) defines a C99 API for identifying prime numbers and performing prime factorization; [primenum.c](primenum.c) implements it using trial division, the inefficient but easy to understand algorithm that every first-time programmer uses. A support module, [list.c](list.c), provides the linked-list type used to return found primes and factors.

[primes.c](primes.c) is a prime number sieve. It prints found primes to `stdout`, and can optionally save them to disk for later use. You can set it to stop after a certain value or number of primes found, or let it keep going until it overflows or runs out of memory.

[pfactor.c](pfactor.c) is a prime factorization tool. It prints the prime factors of values passed on its command line to `stdout`.

I wrote this stuff for my own amusement and practice working in C. It may contain clumsy implementations, faulty assumptions, and fundamentally dodgy math; it almost certainly includes a decent number of bugs. This is not intended as production-ready code, and I take no responsibility for how you might choose to use it.

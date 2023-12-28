# Dynamic Array

An implementation of a dynamic array in C as a single header-file library.
This implementation aims to act like a c++ vector or an ArrayList, where the array's
items are layed out contiguously in memory.

Because generics are not "first-class" in C, workarounds are needed to make
this data structure work with different datatypes. Several were considered, and none of them are ideal:

1. Manually implement a dynamic array everywhere it is used
    * Pros: actual type is known (so items can be passed by value), only as complicated as needed
    * Cons:
        * **Effort:** A lot of work, which really should be avoided
        * **Mistakes:** Programmer is prone to make mistakes (I definitely didn't get this right first try, thanks testing)
        * **Too much code:** Little code reuse, increasing the amount of code needing to be read by the programmer, and increasing binary size
2. Have obfuscated generics with void pointers by storing the size of the item at runtime
    * Pros: code is fully reusable only needing a single implementation, type size is specified once at initialization
    * Cons:
        * **Slower:** Pointer offsets calculated at runtime
        * **More RAM:** Needs more memory to store the item size
        * **Sometimes inconvenient:** Needs casts from void*, so cannot pass items by value
3. Use multi-line macros to generate implementations for a specific type ([example](https://codereview.stackexchange.com/questions/259374/dynamic-generic-array-list-implementation-in-c99))
    * Pros: actual type is known (so items can be passed by value), code is reused by the programmer
    * Cons:
        * **Syntax change:** Type name must be typed every time the array implementation is used, as a macro argument
        * **Too much code:** Every generic function operating on the array is inlined, increasing binary size

I chose #2 because it maximizes code reuse while keeping binary size low and preserving the C language syntax/semantics.
Depending on your needs and preferences, you may end up choosing a different solution.

## Basic Usage

hello_dynarr.c
```c
#include <stdio.h>

#define DYNARR_IMPLEMENTATION
#include "dynarr.h"

int main() {
    DynArr a = {sizeof(char)};

    dynarr_append_n(&a, 5, "world"); // "world
    dynarr_insert_n(&a, 0, 7, "hello, "); // "hello, world
    dynarr_append_n(&a, 2, "!"); // "hello, world!"
    printf("%s\n", (char*)a.arr);

    dynarr_pop_n(&a, 7); // "hello, 
    if(dynarr_reserve(&a, 1024)) { // Allocate a lot more space for whatever the user will input
        char buf[1024] = {0};
        printf("Enter your name: ");
        scanf("%s", buf);
        dynarr_append_n(&a, strlen(buf), buf);
        dynarr_append_n(&a, 2, "!"); // "hello, _name_!"
        printf("%s\n", (char*)a.arr);
    }
    dynarr_free(&a);
}
```
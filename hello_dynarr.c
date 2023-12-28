#include <stdio.h>
#include <ctype.h>

#define DYNARR_IMPLEMENTATION
#include "dynarr.h"

int main() {
    DynArr a = {sizeof(char)};

    dynarr_append_n(&a, 5, "world"); // "world
    dynarr_insert_n(&a, 0, 7, "hello, "); // "hello, world
    dynarr_append_n(&a, 2, "!"); // "hello, world!"
    printf("%s\n", (char*)a.arr);

    dynarr_pop_n(&a, 7); // "hello, 
    printf("Enter your name: ");
    while (1) {
        char c = getchar();
        if (c == EOF || isspace(c)) { break; }
        dynarr_append(&a, &c);
    }
    dynarr_append_n(&a, 2, "!"); // "hello, _name_!"
    printf("%s\n", (char*)a.arr);
    printf("Length: %ld, Capacity: %ld\n", a.len, a.cap);
    dynarr_free(&a);
}
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
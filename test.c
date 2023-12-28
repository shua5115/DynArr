#include <stdio.h>
#include <assert.h>

#define DYNARR_IMPLEMENTATION
#include "dynarr.h"

typedef void (*test_fn)(void);

int is_arr_eq(const int *a, const int *b, size_t n) {
    if (a == NULL || b == NULL) return 0;
    for(size_t i = 0; i < n; i++) {
        if(a[i] != b[i]) return 0;
    }
    return 1;
}

void print_arr(const int *a, size_t n) {
    printf("{");
    for(size_t i = 0; i < n; i++) {
        printf("%d",a[i]);
        if(i < n-1) printf(", ");
    }
    printf("}\n");
}

void test_alloc_reserve();
void test_append_insert();
void test_remove_pop();
void test_mem();

test_fn tests[] = {
    test_alloc_reserve,
    test_append_insert,
    test_remove_pop,
    test_mem
};

int main() {
    for(int i = 0; i < sizeof(tests)/sizeof(test_fn); i++) {
        test_fn test = tests[i];
        test();
    }
    printf("All tests passed!\n");
}

void test_alloc_reserve() {
    DynArr a = {sizeof(int)};
    dynarr_ensure(&a, 69);
    assert(a.cap >= 69 && a.len == 0 && a.arr != NULL);
    dynarr_reserve(&a, 5);
    assert(a.cap >= 69+5 && a.len == 0);
    dynarr_free(&a);
    assert(a.cap == 0);
}

void test_append_insert() {
    const int target1[5] = {1, 2, 3, 4, 5};
    DynArr a = {sizeof(int)};
    int *result = dynarr_append_n(&a, 5, target1);
    assert(result != NULL);
    assert(a.len == 5);
    assert(a.cap >= a.len);
    assert(is_arr_eq(a.arr, target1, a.len));

    const int change[5] = {69, 70, 71, 72, 73};
    const int target2[10] = {1, 2, 3, 69, 70, 71, 72, 73, 4, 5};
    result = dynarr_insert_n(&a, 3, 5, change);
    assert(result!=NULL);
    assert(a.len == 10);
    assert(is_arr_eq(a.arr, target2, a.len));

    dynarr_clear(&a);
    assert(a.len == 0 && a.cap != 0);
    
    dynarr_free(&a);
}

void test_remove_pop() {
    const int target1[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    DynArr a = {sizeof(int)};
    dynarr_append_n(&a, 9, target1);
    assert(is_arr_eq(a.arr, target1, a.len));

    const int target2[7] = {1, 2, 5, 6, 7, 8, 9};
    dynarr_remove_n(&a, 2, 2);
    assert(a.len == 7);
    assert(is_arr_eq(a.arr, target2, a.len));

    const int target3[4] = {7, 8, 9, 6};
    dynarr_swap_remove_n(&a, 0, 3);
    assert(a.len == 4);
    assert(is_arr_eq(a.arr, target3, a.len));

    const int target4[2] = {7, 8};
    dynarr_pop_n(&a, 2);
    assert(a.len == 2);
    assert(is_arr_eq(a.arr, target4, a.len));

    dynarr_free(&a);
}

void test_mem() {
    const int target1[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    DynArr a = {sizeof(int)};
    dynarr_append_n(&a, 9, target1);
    assert(is_arr_eq(a.arr, target1, a.len));

    int val = *((int*)dynarr_get(&a, 6));
    assert(val == 7);

    assert(dynarr_raw(&a, int) == a.arr);

    dynarr_clear(&a);
    dynarr_shrink(&a, 3);
    assert(a.cap >= 3 && a.cap < 9);
    dynarr_shrink(&a, 0);
    assert(a.cap < 3);

    dynarr_free(&a);
}
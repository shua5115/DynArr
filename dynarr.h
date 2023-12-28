/*
Copyright 2023 Yehoshua Halle <yehoshua.halle@outlook.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef DYNARR_H
#define DYNARR_H

#include <stddef.h>

// Contains a contiguous, growable array of items, where each item is of size itemsize.
// Initialize as {sizeof(Type)}, where type is the value to be stored in the array
typedef struct DynArr {
    size_t itemsize;
    void *arr;
    size_t len;
    size_t cap;
} DynArr;

// Gets the underlying array held by the dynamic array as a specific type
#define dynarr_raw(ptr_to_dynarr, itemtype) ((itemtype*)(ptr_to_dynarr)->arr)

// Frees all memory associated with the dynamic array, and resets length and capacity.
void dynarr_free(DynArr *arr);

// Bounds checked index into the array. Returns NULL if out of bounds.
void *dynarr_get(DynArr *arr, size_t idx);

// Ensures that the dynamic array has the capacity to hold at least n items.
// Returns the address of the underlying array on success, or NULL on failure.
void *dynarr_ensure(DynArr *arr, size_t n);

// Reserves enough capacity for n additional items.
// Returns the address of the underlying array on success, or NULL on failure.
void *dynarr_reserve(DynArr *arr, size_t n);

// Appends n items to the end of the array.
// Returns the address of the newly appended items. Returns NULL if unsuccessful.
// If items is NULL, then n new items are allocated in the array as zeros.
void *dynarr_append_n(DynArr *arr, size_t n, const void *items);
#define dynarr_append(arr, item) dynarr_append_n((arr), 1, (item))

// Inserts n items inside the array, shifting items to the right to make space.
// Returns the address of the insertion. Returns NULL if unsuccessful.
// If items is NULL, then n new items are allocated in the array as zeros.
void *dynarr_insert_n(DynArr *arr, size_t idx, size_t n, const void *items);
#define dynarr_insert(arr, idx, item) dynarr_insert_n((arr), (idx), 1, (item))

// Removes n items from the array starting at idx, and shifts all following elements to fill the gap.
void dynarr_remove_n(DynArr *arr, size_t idx, size_t n);
#define dynarr_remove(arr, idx) dynarr_remove_n((arr), (idx), 1)

// Removes n items from the back of the array
void dynarr_pop_n(DynArr *arr, size_t n);
#define dynarr_pop(arr) dynarr_pop_n((arr), 1)

// Removes one item from the array at idx, moving the last item into its place.
void dynarr_swap_remove_n(DynArr *arr, size_t idx, size_t n);
#define dynarr_swap_remove(arr, idx) dynarr_swap_remove_n((arr), (idx), 1)

// Removes all elements from the dynamic array without performing any allocations.
void dynarr_clear(DynArr *arr);

// Frees memory used by the dynamic array to store at least min_cap items.
// min_cap is at least 1 to ensure that the internal array always points to valid memory before the array is free'd.
void dynarr_shrink(DynArr *arr, size_t min_cap);

#endif

// Implementation
#ifdef DYNARR_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

void dynarr_free(DynArr *arr) {
    free(arr->arr);
    arr->arr = NULL;
    arr->len = 0;
    arr->cap = 0;
}

void *dynarr_get(DynArr *arr, size_t idx) {
    if(arr == NULL || idx >= arr->len) { return NULL; }
    return ((char*)arr->arr) + idx*arr->itemsize;
}

void *dynarr_ensure(DynArr *arr, size_t n) {
    // if(arr==NULL) { return NULL; } // remove often redundant NULL check, callers responsible
    if (n <= arr->cap) { return arr->arr; }
    size_t newcap = (arr->cap == 0) ? 1 : arr->cap;
    while(newcap < n) {
        newcap <<= 1; // Multiply by 2 until larger
    }
    void *newarr = reallocarray(arr->arr, newcap, arr->itemsize);
    if (newarr==NULL) { return NULL; }
    arr->arr = newarr;
    arr->cap = newcap;
    return newarr;
}

void *dynarr_reserve(DynArr *arr, size_t n) {
    // if(arr==NULL) { return NULL; } // remove often redundant NULL check, callers responsible
    return dynarr_ensure(arr, arr->len+n);
}

void *dynarr_append_n(DynArr *arr, size_t n, const void *items) {
    if (arr == NULL) { return NULL; }
    char *ptr = (char*) dynarr_reserve(arr, n);
    if (ptr == NULL) { return NULL; }
    ptr += arr->len*arr->itemsize; // ptr now points to start of items
    if (items != NULL) {
        memcpy(ptr, items, n*arr->itemsize);
    } else {
        memset(ptr, 0, n*arr->itemsize);
    }
    arr->len += n;
    return ptr;
}

void *dynarr_insert_n(DynArr *arr, size_t idx, size_t n, const void *items) {
    if (arr == NULL || idx >= arr->len) { return NULL; }
    if (n == 0) { return arr->arr; }
    char *ptr = (char*) dynarr_reserve(arr, n);
    if (ptr == NULL) { return NULL; }
    ptr += idx*arr->itemsize; // ptr now points to start of items
    size_t bytes = n*arr->itemsize;
    if (idx < arr->len) {
        memmove(ptr+bytes, ptr, (arr->len-idx)*arr->itemsize);
    }
    if (items != NULL) {
        memcpy(ptr, items, bytes);
    } else {
        memset(ptr, 0, bytes);
    }
    arr->len += n;
    return ptr;
}

void dynarr_remove_n(DynArr *arr, size_t idx, size_t n) {
    if (arr == NULL || n == 0 || idx >= arr->len || arr->arr == NULL) { return; }
    if (idx+n >= arr->len) {
        // Then we can just shrink the array size without moving any memory
        if (idx+n > arr->len) { n = arr->len - idx; } // Make sure array size is clamped to zero
        arr->len -= n;
        return;
    }
    // Remove items between arr+idx and arr+idx+n, safe to do since arr+idx+n is at most the final value of the array
    char* carr = (char*) arr->arr;
    memmove(carr+idx*arr->itemsize, carr+(idx+n)*arr->itemsize, (arr->len-idx)*arr->itemsize);
    arr->len -= n;
}

void dynarr_pop_n(DynArr *arr, size_t n) {
    if(n >= arr->len) {
        arr->len = 0;
    } else {
        arr->len -= n;
    }
}

void dynarr_swap_remove_n(DynArr *arr, size_t idx, size_t n) {
    if (arr == NULL || n == 0 || arr->len == 0 || idx >= arr->len || arr->arr == NULL) { return; }
    if (idx+n > arr->len) { n = arr->len - idx; }
    arr->len -= n;
    char* carr = (char*) arr->arr;
    if (idx+n < arr->len) {
        memcpy(carr+idx*arr->itemsize, carr+arr->len*arr->itemsize, n*arr->itemsize);
    }
}

void dynarr_clear(DynArr *arr) {
    arr->len = 0;
}

void dynarr_shrink(DynArr *arr, size_t min_cap) {
    if(arr == NULL || arr->cap == 0) { return; }
    size_t newcap = 1;
    while(newcap < min_cap || newcap < arr->len) {
        newcap <<= 1;
    }
    void *newarr = reallocarray(arr->arr, newcap, arr->itemsize);
    if (newarr != NULL) {
        arr->arr = newarr;
        arr->cap = newcap;
    }
}

#endif

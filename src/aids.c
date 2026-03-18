#define AIDS_IMPLEMENTATION
#include "aids.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void* dmalloc(Allocator* _, size_t size) {
    return malloc(size);
}
void* drealloc(Allocator* _, void* ptr, size_t size) {
    return realloc(ptr, size);
}
void* dcalloc(Allocator* _, size_t nmemb, size_t size) {
    return calloc(nmemb, size);
}
void dfree(Allocator* _, void* ptr) {
    free(ptr);
}

Allocator DEFAULT_ALLOCATOR = {
    .alloc = dmalloc,
    .realloc = drealloc,
    .calloc = dcalloc,
    .free = dfree,
};

void* tracking_alloc_i(TrackingAllocator* self, size_t size) {
    void* ptr = ALLOC(self->allocator, size);
    da_append(&self->pointers, ptr);
    return ptr;
}

void* tracking_calloc_i(TrackingAllocator* self, size_t nmemb, size_t size) {
    void* ptr = CALLOC(self->allocator, nmemb, size);
    da_append(&self->pointers, ptr);
    return ptr;
}

void* tracking_realloc_t(TrackingAllocator* self, void* ptr, size_t size) {
    if(ptr == NULL) return ALLOC(self, size);
    da_foreach(pointer, &self->pointers) {
        //pointer is a void**
        if((*pointer) == ptr) {
            *pointer = REALLOC(self->allocator, ptr, size);
            return *pointer;
        }
    }
    panic("the pointer isn't in the Tracking Allocator\n");
}

void tracking_free_i(TrackingAllocator* self, void* ptr) {
    if(ptr == NULL) return;
    da_foreach(pointer, &self->pointers) {
        //pointer is a void**
        if((*pointer) == ptr) {
            FREE(self->allocator, ptr);
            *pointer = NULL;
            return;
        }
    }
    panic("the pointer isn't in the Tracking Allocator\n");
}

TrackingAllocator tracking_create(Allocator* backing) {
    TrackingAllocator a = {0};
    a.allocator = backing;
    da_init(&a.pointers, a.allocator);
    a.alloc = (fun_alloc) tracking_alloc_i;
    a.calloc = (fun_calloc) tracking_calloc_i;
    a.free = (fun_free) tracking_free_i;
    a.realloc = (fun_realloc) tracking_realloc_t;
    return a;
}

void tracking_destroy(TrackingAllocator* self) {
    da_foreach(ptr, &self->pointers) {
        FREE(self->allocator, *ptr); 
    }
    da_free(&self->pointers);
}

String string_from(const char* str, Allocator* alloc) {
    size_t l = strlen(str);
    if(l == 0) {
        return (String) {0};
    }
    String string = {0};
    string.alloc = alloc;
    string.data = ALLOC(alloc,l+1);
    strcpy(string.data, str);
    string.cap = l+1;
    string.len = l;
    return string;
}
String string_clone(String * str, Allocator* alloc) {
    return da_clone(str, alloc);
}
char* from_string(String* str, Allocator* alloc) {
    char* s = ALLOC(alloc, str->len + 1);
    memcpy(s, str->data, str->len);
    s[str->len] = '\0';
    return s;
}
void append_char(String* str, char ch) {
    da_append(str, ch);
}
void append_cstr(String* str, char* s) {
    size_t len = strlen(s); 

    for(size_t i = 0; i < len; i++) {
        da_append(str, s[i]);
    }
}
void append_string(String* str, String* val) {
    da_foreach(c, val) {
        da_append(str, *c);
    }
}

bool string_compare(String a, String b) {
    if(a.len != b.len) return false;
    for(size_t i = 0; i < a.len; i++) {
        if(a.data[i] != b.data[i]) return false;
    }
    return true;
}

bool str_cmp(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

String string_create(Allocator* alloc) {
    String s = {0};
    da_init(&s, alloc);
    return s;
}

size_t hm_hash_cstr(const char* s) {
    size_t hash = 1469598103934665603ULL;
    while (*s) {
        hash ^= (unsigned char)*s++;
        hash *= 1099511628211ULL;
    }
    return hash;
}

size_t hm_hash_int(int s) {
    return s;
}

size_t hm_hash_string(String s) {
    size_t hash = 1469598103934665603ULL;
    da_foreach(ch, &s) {
        hash ^= (unsigned char)*ch;
        hash *= 1099511628211ULL;
    }
    return hash;
}

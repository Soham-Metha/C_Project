#pragma once
#include <Utils/string_view.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGION_DEFAULT_CAPACITY 65536

typedef struct Region Region;
typedef struct Arena Arena;

struct Region {
    Region* next;
    size_t capacity;
    size_t size;
    char buffer[];
};

struct Arena {
    Region* first;
    Region* last;
};

Region* region_create(size_t capacity)
{
    const size_t partSize = sizeof(Region) + capacity;
    Region* part          = (Region*)malloc(partSize);
    memset(part, 0, partSize);
    part->capacity = capacity;
    return part;
}

char* arena_insert_or_expand(Arena* arena, Region* cur, size_t size, size_t aligned_address_mask)
{
    uintptr_t tmp   = (uintptr_t)(cur->buffer + cur->size);
    tmp             = (tmp + aligned_address_mask) & ~aligned_address_mask;

    char* ptr       = (char*)tmp;
    size_t realSize = (ptr + size) - (cur->buffer + cur->size);

    if (cur->size + realSize <= cur->capacity) {
        memset(ptr, 0, realSize);
        cur->size += realSize;
        return ptr;
    }

    if (cur->next) {
        return arena_insert_or_expand(arena, cur->next, size, aligned_address_mask);
    }

    size_t worstCase = (size + aligned_address_mask) & ~aligned_address_mask;

    Region* part     = region_create(
        worstCase > REGION_DEFAULT_CAPACITY
            ? worstCase
            : REGION_DEFAULT_CAPACITY);

    cur               = arena->last;
    arena->last->next = part;
    arena->last       = part;

    return arena_insert_or_expand(arena, cur->next, size, aligned_address_mask);
}

void* region_alloc_aligned(Arena* arena, size_t size, size_t alignment)
{
    if (arena->last == NULL) {
        assert(arena->first == NULL);

        Region* part = region_create(
            size > REGION_DEFAULT_CAPACITY ? size : REGION_DEFAULT_CAPACITY);

        arena->last  = part;
        arena->first = part;
    }

    if (size == 0) {
        return arena->last->buffer + arena->last->size;
    }

    assert((alignment & (alignment - 1)) == 0);

    return arena_insert_or_expand(arena, arena->last, size, alignment - 1);
}

void* region_alloc(Arena* arena, size_t size)
{
    return region_alloc_aligned(arena, size, sizeof(void*));
}

const char* arena_sv_to_cstr(Arena* arena, String_View str)
{
    char* cstr = (char*)region_alloc(arena, str.len + 1);
    memcpy(cstr, str.data, str.len);
    cstr[str.len] = '\0';
    return cstr;
}

String_View arena_cstr_concat(Arena* arena, const char* a, const char* b)
{
    const size_t aLen = strlen(a);
    const size_t bLen = strlen(b);
    char* buf         = (char*)region_alloc(arena, aLen + bLen);
    memcpy(buf, a, aLen);
    memcpy(buf + aLen, b, bLen);
    return (String_View) {
        .len  = aLen + bLen,
        .data = buf
    };
}

void arena_clear(Arena* arena)
{
    for (Region* part = arena->first; part != NULL; part = part->next) {
        part->size = 0;
    }
    arena->last = arena->first;
}

void arena_free(Arena* arena)
{
    for (Region *part = arena->first, *next = NULL; part != NULL; part = next) {
        next = part->next;
        free(part);
    }
}

/**
 * @file dynlist.h
 * @author Lungile Maseko 
 * @brief This file contains an implementation of a dynamic list
 * 
 * 
 * @details
 * Memory layout:
 * - u64 capacity = number elements that can be held.
 * - u64 length = number of elements currently contained
 * - u64 stride = size of each element in bytes
 * - void* elements
 * 
 * @version 0.1
 * @date 2023-10-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once

#include "defines.h"

#define DYNLIST_DEFAULT_CAPACITY 1
#define DYNLIST_RESIZE_FACTOR 2

//Private Functions
void* _dynlist_create(u64 length, u64 stride);
void* _dynlist_resize(void* list);
void* _dynlist_push(void* list, const void* value_ptr);
void* _dynlist_insert_at(void* list, u64 index, void* value_ptr);
//Public Functions
void dynlist_pop(void* list, void* value_ptr);
void* dynlist_pop_at(void* list, u64 index, void* value_ptr);
void dynlist_destroy(void* list);
void dynlist_clear(void* list);
u64 dynlist_capacity(void* list);
u64 dynlist_length(void* list);
u64 dynlist_stride(void* list);
void dynlist_length_set(void* list, u64 value);

#define dynlist_create(type) \
    _dynlist_create(DYNLIST_DEFAULT_CAPACITY, sizeof(type))

#define dynlist_reserve(type, capacity) \
    _dynlist_create(capacity, sizeof(type))

#define dynlist_push(list, value)           \
    {                                       \
        typeof(value) temp = value;         \
        list = _dynlist_push(list, &temp);  \
    }

#define dynlist_insert_at(list, index, value)           \
    {                                                   \
        typeof(value) temp = value;                     \
        list = _dynlist_insert_at(list, index, &temp); \
    }





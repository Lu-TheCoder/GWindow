#include "dynlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dynlist_header{
    u64 capacity;
    u64 length;
    u64 stride;
}dynlist_header;

void* _dynlist_create(u64 length, u64 stride){
    u64 header_size = sizeof(dynlist_header);
    u64 list_size = length * stride;
    void* new_list = 0;
    new_list = malloc(header_size + list_size);
    memset(new_list, 0, header_size + list_size);

    if(length == 0){
        printf("dynlist_create() called with a length of 0.\n");
    }

    dynlist_header* header = new_list;
    header->capacity = length;
    header->length = 0;
    header->stride = stride;

    return (void*)((u8*)new_list + header_size);
}

void dynlist_destroy(void* list){
    if (list) {
        u64 header_size = sizeof(dynlist_header);
        dynlist_header* header = (dynlist_header*)((u8*)list - header_size);
        // u64 total_size = header_size + header->capacity * header->stride;
        free(header);
    }
}


void* _dynlist_resize(void* list){
    u64 header_size = sizeof(dynlist_header);
    dynlist_header* header = (dynlist_header*)((u8*)list - header_size);
    if (header->capacity == 0) {
        printf("_dynlist_resize called on an list with 0 capacity. This should not be possible.\n");
        return 0;
    }
    void* temp = _dynlist_create((DYNLIST_RESIZE_FACTOR * header->capacity), header->stride);

    header = (dynlist_header*)((u8*)list - header_size);
    memcpy(temp, list, header->length * header->stride);

    dynlist_length_set(temp, header->length);
    dynlist_destroy(list);
    return temp;
}

void* _dynlist_push(void* list, const void* value_ptr){
    u64 header_size = sizeof(dynlist_header);
    dynlist_header* header = (dynlist_header*)((u8*)list - header_size);
    if (header->length >= header->capacity) {
        list = _dynlist_resize(list);
    }
    header = (dynlist_header*)((u8*)list - header_size);

    u64 addr = (u64)list;
    addr += (header->length * header->stride);
    memcpy((void*)addr, value_ptr, header->stride);
    dynlist_length_set(list, header->length + 1);
    return list;
}

void* _dynlist_insert_at(void* list, u64 index, void* value_ptr){
    u64 length = dynlist_length(list);
    u64 stride = dynlist_stride(list);
    if (index >= length) {
        printf("Index outside the bounds of this list! Length: %llu, index: %llu \n", length, index);
        return list;
    }
    if (length >= dynlist_capacity(list)) {
        list = _dynlist_resize(list);
    }

    u64 addr = (u64)list;

    // If not on the last element, copy the rest outward.
    if (index != length - 1) {
        memcpy(
            (void*)(addr + ((index + 1) * stride)),
            (void*)(addr + (index * stride)),
            stride * (length - index));
    }

    // Set the value at the index
    memcpy((void*)(addr + (index * stride)), value_ptr, stride);

    dynlist_length_set(list, length + 1);
    return list;
}

void dynlist_pop(void* list, void* value_ptr){
    u64 length = dynlist_length(list);
    u64 stride = dynlist_stride(list);
    u64 addr = (u64)list;
    addr += ((length - 1) * stride);
    memcpy(value_ptr, (void*)addr, stride);
    dynlist_length_set(list, length - 1);
}

void* dynlist_pop_at(void* list, u64 index, void* value_ptr){
    u64 length = dynlist_length(list);
    u64 stride = dynlist_stride(list);
    if (index >= length) {
        printf("Index outside the bounds of this list! Length: %llu, index: %llu\n", length, index);
        return list;
    }

    u64 addr = (u64)list;
    memcpy(value_ptr, (void*)(addr + (index * stride)), stride);

    // If not on the last element, snip out the entry and copy the rest inward.
    if (index != length - 1) {
        memcpy(
            (void*)(addr + (index * stride)),
            (void*)(addr + ((index + 1) * stride)),
            stride * (length - index));
    }

    dynlist_length_set(list, length - 1);
    return list;
}


void dynlist_clear(void* list){
    dynlist_length_set(list, 0);
}

u64 dynlist_capacity(void* list){
    u64 header_size = sizeof(dynlist_header);
    dynlist_header* header = (dynlist_header*)((u8*)list - header_size);
    return header->capacity;
}

u64 dynlist_length(void* list){
    u64 header_size = sizeof(dynlist_header);
    dynlist_header* header = (dynlist_header*)((u8*)list - header_size);
    return header->length;
}

u64 dynlist_stride(void* list){
    u64 header_size = sizeof(dynlist_header);
    dynlist_header* header = (dynlist_header*)((u8*)list - header_size);
    return header->stride;
}

void dynlist_length_set(void* list, u64 value){
    u64 header_size = sizeof(dynlist_header);
    dynlist_header* header = (dynlist_header*)((u8*)list - header_size);
    header->length = value;
}
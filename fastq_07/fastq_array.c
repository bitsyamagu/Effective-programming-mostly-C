#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "fastq_array.h"

// assumes that payload is pointers
void FastQArray_init(FastQArray* p){
    p->buf = malloc(sizeof(uint64_t*)*INITIAL_FQ_ARRAY_CAPACITY);
    p->capacity = INITIAL_FQ_ARRAY_CAPACITY;
    p->length = 0;
}
void FastQArray_ensure(FastQArray* p, int index){
    if(index >= p->capacity){
        int new_capacity = p->capacity * 2;
        p->buf = realloc(p->buf, sizeof(uint64_t*)*new_capacity);
        // fprintf(stderr, "old: %d, new %d\n", p->capacity, new_capacity);
        p->capacity = new_capacity;
    }
}
void FastQArray_push_back(FastQArray* p, void* fq){
    FastQArray_ensure(p, p->length+1);
    p->buf[p->length] = fq;
    p->length++;
}
/*
int FastQArray_parse_index(char* a){
    char dummy[256];
    int a_num = 0;
    int a_den = 0;
    int a_count = sscanf(a, "%s %d/%d", dummy, &a_num, &a_den);
    assert(a_count == 3);
    return a_num;
}*/
// private
void bubble_sort(FastQArray* array, int(*usr_cmp_func)(const void*, const void*)){
    int swapped = 1;
    while(swapped){
        swapped = 0;
        for(int i = 0; i<array->length-1; i++){
            if(usr_cmp_func(array->buf[i], array->buf[i+1]) > 0){
                void* tmp = array->buf[i];
                array->buf[i] = array->buf[i+1];
                array->buf[i+1] = tmp;
                swapped = 1;
            }
        }
    }
}

void FastQArray_sort(FastQArray* array, int(*usr_cmp_func)(const void*, const void*)){
    bubble_sort(array, usr_cmp_func);
    //  qsort(array->buf, array->length, sizeof(FastQ*), usr_cmp_func);
}

void FastQArray_finish(FastQArray* p){
    for(int i = 0; i<p->length; i++){
        free(p->buf[p->length]);
    }
    free(p->buf);
}


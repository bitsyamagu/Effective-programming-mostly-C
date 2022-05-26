#include<string.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct {
    char name[256];
    char seq[256];
    char qual[256];
} FastQ;

#define INITIAL_FQ_ARRAY_CAPACITY 2
typedef struct {
    FastQ** buf;
    int capacity;
    int length;
} FastQArray;

void FastQArray_init(FastQArray* p){
    p->buf = (FastQ**)malloc(sizeof(FastQ*)*INITIAL_FQ_ARRAY_CAPACITY);
    p->capacity = INITIAL_FQ_ARRAY_CAPACITY;
    p->length = 0;
}
void FastQArray_ensure(FastQArray* p, int index){
    if(index >= p->capacity){
        int new_capacity = p->capacity * 2;
        p->buf = (FastQ**)realloc(p->buf, sizeof(FastQ*)*new_capacity);
        fprintf(stderr, "old: %d, new %d\n", p->capacity, new_capacity);
        p->capacity = new_capacity;
    }
}
void FastQArray_push_back(FastQArray* p, FastQ* fq){
    FastQArray_ensure(p, p->length+1);
    p->buf[p->length] = fq;
    p->length++;
}
void FastQArray_finish(FastQArray* p){
    for(int i = 0; i<p->length; i++){
        free(p->buf[p->length]);
    }
    free(p->buf);
}

int main(int argc, char** argv){
    FILE* fp = fopen(argv[1], "r");
    char buf[256];
    FastQArray array;
    FastQArray_init(&array);

    int ok = 1;
    do {
        // name
        char* p = fgets(buf, 256, fp);
        if(p == NULL){
            ok = 0;
        }else {
            FastQ* fq = (FastQ*)malloc(sizeof(FastQ));

            strcpy(fq->name, buf);
            // seq
            fgets(buf, 256, fp);
            strcpy(fq->seq, buf);
            // +
            fgets(buf, 256, fp);
            // qual
            fgets(buf, 256, fp);
            strcpy(fq->qual, buf);

            FastQArray_push_back(&array, fq);
        }
    }while(ok);

    for(int i = 0; i<array.length; i++){
        printf("%s", array.buf[i]->name);
        printf("%s", array.buf[i]->seq);
        printf("+\n");
        printf("%s", array.buf[i]->qual);
    }

    FastQArray_finish(&array);

    return 0;
}

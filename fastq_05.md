これまで見てきたFastQファイルはリード長は最大255塩基としてきました。
一方、実際のshort readのシーケンスは150塩基以下のデータが多いので、
約40%ほどメモリを無駄に消費していることになります。またリード名は
それよりも短いことも多いので、50%程度は無駄かも知れません。

この無駄を無くすには以下のようなFastQ構造体を
```C
typedef struct {
    char name[256];
    char seq[256];
    char qual[256];
} FastQ;
```
次のように修正して、任意の長さの文字列へのポインタとして、
個別にメモリをアロケーションしてやるのが最も効率的です。
```C
typedef struct {
    char* name;
    char* seq;
    char* qual;
} FastQ;
```

main6.c
```C
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct {
    char* name;
    char* seq;
    char* qual;
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
FastQ* create_FastQ(char* name, char* seq, char* qual){
    FastQ* p = (FastQ*)malloc(sizeof(FastQ));
    int name_len = strlen(name);
    int seq_len = strlen(seq);
    int qual_len = strlen(qual);
    p->name = (char*) malloc(sizeof(char)*name_len);
    p->seq = (char*) malloc(sizeof(char)*seq_len);
    p->qual = (char*) malloc(sizeof(char)*qual_len);
    strncpy(p->name, name, name_len-1);
    strncpy(p->seq, seq, seq_len-1);
    strncpy(p->qual, qual, qual_len-1);
    p->name[name_len] = '\0';
    p->seq[seq_len] = '\0';
    p->qual[qual_len] = '\0';

    return p;
}

int main(int argc, char** argv){
    FILE* fp = fopen(argv[1], "r");
    char buf_name[256];
    char buf_seq[256];
    char buf_qual[256];
    FastQArray array;
    FastQArray_init(&array);

    while(1) {
        // name
        char* p = fgets(buf_name, 256, fp);
        if(p == NULL){
            break;
        }
        fgets(buf_seq, 256, fp);
        fgets(buf_qual, 256, fp); // dummy
        fgets(buf_qual, 256, fp);
        FastQ* fq = create_FastQ(buf_name, buf_seq, buf_qual);
        FastQArray_push_back(&array, fq);
    }

    for(int i = 0; i<array.length; i++){
        printf("%s\n", array.buf[i]->name);
        printf("%s\n", array.buf[i]->seq);
        printf("+\n");
        printf("%s\n", array.buf[i]->qual);
    }
    FastQArray_finish(&array);

    return 0;
}
```
![img](images/diff_main5_main6.png)

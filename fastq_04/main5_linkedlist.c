#include<string.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct {
    char name[256];
    char seq[256];
    char qual[256];
} FastQ;

typedef struct FastQNode {
    FastQ* value;
    struct FastQNode* next;
} FastQNode;

typedef struct {
    FastQNode* head;
    int length;
} FastQLinkedList;

void FastQLinkedList_init(FastQLinkedList* p){
    p->head = NULL;
    p->length = 0;
}
void FastQLinkedList_insert(FastQLinkedList* p, FastQ* fq){
    FastQNode* old_head = p->head;
    p->head = (FastQNode*)malloc(sizeof(FastQNode));
    p->head->value = fq;
    p->head->next = old_head;
    p->length++;
}
void FastQLinkedList_finish(FastQLinkedList* list){
    FastQNode* p = list->head;
    while(p != NULL){
        free(p->value);
        FastQNode* tmp = p;
        p = p->next;
        free(tmp);
    }
}

int main(int argc, char** argv){
    FILE* fp = fopen(argv[1], "r");
    char buf[256];
    FastQLinkedList list;
    FastQLinkedList_init(&list);

    while(1) {
        // name
        char* p = fgets(buf, 256, fp);
        if(p == NULL){
            break;
        }
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

        FastQLinkedList_insert(&list, fq);
    }


    FastQNode* p = list.head;
    while(p != NULL){
        printf(": %s", p->value->name);
        p = p->next;
//      printf("%s", list.buf[i]->seq);
//      printf("+\n");
//      printf("%s", list.buf[i]->qual);
    }
    FastQLinkedList_finish(&list);

    return 0;
}

前回見たFastQArrayはArrayと呼ばれる可変長配列で、その実体はポインタの配列に皮を一枚かぶせたものでした。そしてそのような一次元の配列の
弱点として、時々メモリの再アロケーションと中身のコピーが必要となっており、規模の大きな配列になるほどそのコストも増大するという問題がありました。
特に配列の先頭に1要素を挿入しようとすると配列全体をシフトする必要があるので、巨大な配列の先頭への挿入は非常に非効率的なものになってしまいます。

今回紹介するLinkedListはそのようなコストのかからないデータ構造の一つです。以下の図は、空っぽの初期状態から、一個ずつ要素を
追加する様子を示したものです。

![img](images/linked_list.png)

これによく似た現実のものとしては貨物列車があります。先頭の緑色の構造体は下記のコードではFastQLinkedListがこれにあたります。
これは貨物列車の機関車です。そして、青いノードを追加するのは貨物車両を後ろに連結します。積載物に相当するものが図の中のvalueです。
nextには次のノードへのポインタが格納されていて、これを先頭から順に辿ることで末尾のノードまでアクセスできる仕組みです。
```
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct {
    char name[256];
    char seq[256];
    char qual[256];
} FastQ;

#define INITIAL_FQ_ARRAY_CAPACITY 2

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
        printf("%s", list.buf[i]->seq);
        printf("+\n");
        printf("%s", list.buf[i]->qual);
    }
    FastQLinkedList_finish(&list);

    return 0;
}
```

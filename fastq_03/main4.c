#include<string.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct {
    char name[256];
    char seq[256];
    char qual[256];
} FastQ;

int main(int argc, char** argv){
    FILE* fp = fopen(argv[1], "r");
    char buf[256];
    FastQ** list = (FastQ**)malloc(sizeof(FastQ*)*2);
    int capacity = 2;
    int length = 0;
    while(1) {
        // name
        char* p = fgets(buf, 256, fp);
        if(p == NULL){
            break;
        }
        if(length >= capacity){
            int new_capacity = capacity * 2;
            FastQ** new_list = (FastQ**)malloc(sizeof(FastQ*)*new_capacity);
            memcpy(new_list, list, sizeof(FastQ*) * capacity);
            free(list);
            list = new_list;
            capacity = new_capacity;
        }

        list[length] = (FastQ*)malloc(sizeof(FastQ));

        strcpy(list[length]->name, buf);
        // seq
        fgets(buf, 256, fp);
        strcpy(list[length]->seq, buf);
        // +
        fgets(buf, 256, fp);
        // qual
        fgets(buf, 256, fp);
        strcpy(list[length]->qual, buf);

        length++;
    }
    
    for(int i = 0; i<length; i++){
        printf("%s", list[i]->name);
        printf("%s", list[i]->seq);
        printf("+\n");
        printf("%s", list[i]->qual);
    }

    for(int i = 0; i<length; i++){
        free(list[i]);
    }
    free(list);

    return 0;
}

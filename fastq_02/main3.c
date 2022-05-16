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
    FastQ** list = (FastQ**)malloc(sizeof(FastQ*)*12);
    int index = 0;
    while(1){
        // name
        char* p = fgets(buf, 256, fp);
        if(p == NULL){
            break;
        }
        list[index] = (FastQ*)malloc(sizeof(FastQ));

        strcpy(list[index]->name, buf);
        // seq
        fgets(buf, 256, fp);
        strcpy(list[index]->seq, buf);
        // +
        fgets(buf, 256, fp);
        // qual
        fgets(buf, 256, fp);
        strcpy(list[index]->qual, buf);

        index++;
    }

    for(int i = 0; i<index; i++){
        free(list[i]);
    }
    free(list);

    return 0;
}

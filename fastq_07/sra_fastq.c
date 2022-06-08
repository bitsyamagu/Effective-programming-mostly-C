#include "sra_fastq.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int SRAFastQ_parse_index(char* a){
    char dummy[256];
    int a_num = 0;
    int a_den = 0;
    int a_count = sscanf(a, "@%s %d/%d", dummy, &a_num, &a_den);
    assert(a_count == 3);
    return a_num;
}

int sra_read_comparison(const void* fq1, const void* fq2){
    SRAFastQ* sfq1 = *(SRAFastQ**)fq1;
    SRAFastQ* sfq2 = *(SRAFastQ**)fq2;
    return sfq1->index - sfq2->index;
}

SRAFastQ* readSRAFastQ(FILE* fp){
    char name[256];
    char seq[256];
    char dummy[256];
    char qual[256];
    char* bufp = fgets(name, 255, fp);
    if(bufp == NULL){
        return NULL;
    }
    bufp = fgets(seq, 255, fp);
    bufp = fgets(dummy, 255, fp);
    bufp = fgets(qual, 255, fp);
    SRAFastQ* p = (SRAFastQ*)malloc(sizeof(SRAFastQ));
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
    p->index = SRAFastQ_parse_index(p->name);
    return p;
}

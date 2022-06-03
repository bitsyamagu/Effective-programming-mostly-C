#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sra_fastq.h"
#include "illumina_fastq.h"

#include "fastq_array.h"


int main(int argc, char** argv){
    FILE* fp = fopen(argv[1], "r");
    FastQArray sra;
    FastQArray_init(&sra);
    FastQArray illumina;
    FastQArray_init(&illumina);

    SRAFastQ* sfq = NULL;

    fprintf(stderr, "sra reading\n");
    while(NULL != (sfq = readSRAFastQ(fp))) {
        FastQArray_push_back(&sra, sfq);
    }
    fclose(fp);

    // sort
    fprintf(stderr, "sra sorting\n");
    FastQArray_sort(&sra, sra_read_comparison);

    for(int i = 0; i<sra.length; i++){
        SRAFastQ* fq = (SRAFastQ*)sra.buf[i];
        printf("%s\n", fq->name);
        printf("%s\n", fq->seq);
        printf("+\n");
        printf("%s\n", fq->qual);
    }
    FastQArray_finish(&sra);
    fprintf(stderr, "sra finish\n");

    fprintf(stderr, "illumina reading\n");
    IlluminaFastQ* ifq = NULL;
    fp = fopen(argv[2], "r");
    while(NULL != (ifq = readIlluminaFastQ(fp))) {
        FastQArray_push_back(&illumina, ifq);
    }
    fclose(fp);

    // sort
    fprintf(stderr, "illumina sorting\n");
    FastQArray_sort(&illumina, illumina_read_comparison);
    for(int i = 0; i<illumina.length; i++){
        SRAFastQ* fq = (SRAFastQ*)illumina.buf[i];
        printf("%s\n", fq->name);
        printf("%s\n", fq->seq);
        printf("+\n");
        printf("%s\n", fq->qual);
    }
    FastQArray_finish(&illumina);
    fprintf(stderr, "illumina finish\n");
    return 0;
}

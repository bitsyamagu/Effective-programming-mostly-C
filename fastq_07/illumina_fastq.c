#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "illumina_fastq.h"

#define BUFSIZE 256

char* _read_line(FILE* fp){
    char line[BUFSIZE];
    char* p = fgets(line, 200, fp);
    if(p == NULL){
        return NULL;
    }
    int len = strlen(line);
    char* buf = (char*)malloc(len);
    strncpy(buf, line, len);
    buf[len] = '\0';
    return buf;
}

int illumina_read_comparison(const void* fq1, const void* fq2){
    IlluminaFastQ* p1 = (IlluminaFastQ*)fq1;
    IlluminaFastQ* p2 = (IlluminaFastQ*)fq2;
    if(p1->tile != p2->tile){
        return p1->tile - p2->tile;
    }else if(p1->xpos != p2->xpos){
        return p1->xpos - p2->xpos;
    }else if(p1->ypos != p2->ypos) {
        return  p1->ypos - p2->ypos;
    }
    return 0;
}

void IlluminaFastQ_parse_index(IlluminaFastQ* fq){
    char buf[BUFSIZE];
    uint32_t len = strlen(fq->name);
    strncpy(buf, fq->name, len);
    buf[len] = '\0';

    int32_t col = 0;
    int lane = 0;
    char* p = strtok(buf, ":");

    int tileId = 0;
    while ((p = strtok(NULL, ":# ")) != NULL){
        switch(col) {
            case 3:
                // lane = atoi(p);
                break;
            case 4:
                fq->tile = atoi(p);
                break;
            case 5:
                fq->xpos = atoi(p);
                break;
            case 6:
                fq->ypos = atoi(p);
                break;
            default:
                break;
        }

        col++;
        if(col == UMI){
            break;
        }
    }
}

// from https://support.illumina.com/help/BaseSpace_Sequence_Hub/Source/Informatics/BS/FileFormat_FASTQ-files_swBS.htm
// uint8_t* str = "@SIM:1:FCX:1:15:6329:1045:GATTACT+GTCTTAAC 1:N:0:ATCCGA";
// sim: instrument ID
//   1: run number
// fcx: flow cell ID
//   1: lane number
//  15: tile number
//6329: x coordinate of cluster
//1045: y coordinate of cluster

IlluminaFastQ* readIlluminaFastQ(FILE* fp){
    IlluminaFastQ* p = (IlluminaFastQ*)malloc(sizeof(IlluminaFastQ));
    char* name = _read_line(fp);
    if(name == NULL){
        return NULL;
    }
    char* seq = _read_line(fp);
    char* plus = _read_line(fp); // dummy
    char* qual = _read_line(fp);
    int name_len = strlen(name);
    int seq_len = strlen(seq);
    int qual_len = strlen(qual);
    free(plus);
    p->name = (char*) malloc(sizeof(char)*name_len);
    p->seq = (char*) malloc(sizeof(char)*seq_len);
    p->qual = (char*) malloc(sizeof(char)*qual_len);
    strncpy(p->name, name, name_len-1);
    strncpy(p->seq, seq, seq_len-1);
    strncpy(p->qual, qual, qual_len-1);
    p->name[name_len] = '\0';
    p->seq[seq_len] = '\0';
    p->qual[qual_len] = '\0';
    IlluminaFastQ_parse_index(p);
    return p;
}

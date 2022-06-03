#include <stdio.h>
#include <stdint.h>

enum ILLUMINA_ID {
    INST_ID,
    RUN_NO,
    FCID,
    LANE,
    TILE,
    XPOS,
    YPOS,
    UMI
};

typedef struct {
    char* name;
    char* seq;
    char* qual;
    int16_t tile;
    int16_t xpos;
    int16_t ypos;
} IlluminaFastQ;

void IlluminaFastQ_parse_index(IlluminaFastQ* a);

IlluminaFastQ* readIlluminaFastQ(FILE* fp);

int illumina_read_comparison(const void* p1, const void* p2);

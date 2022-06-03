#include <stdio.h>
#include <stdint.h>

enum ILLUMINA_COL_ID {
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

/**
 * @param  read  parse readname and setup
 */
void IlluminaFastQ_parse_index(IlluminaFastQ* read);

/**
 * @param fp Input file
 * @return FastQ data read from the file
 */
IlluminaFastQ* readIlluminaFastQ(FILE* fp);

/**
 * @param p1 first element for comparison, its type is IlluminaFastQ*
 * @param p2 sedond element for comparison, its type is IlluminaFastQ*
 * @return result of comparison
 */
int illumina_read_comparison(const void* p1, const void* p2);

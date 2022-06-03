#include <stdio.h>

typedef struct {
    char* name;
    char* seq;
    char* qual;
    int index;
} SRAFastQ;

/**
 * @param read_name   Read name
 * @return            Read index
 */
int SRAFastQ_parse_index(char* read_name);

/**
 * @param fp     Input stream
 * @return       New instance of FastQ
 */
SRAFastQ* readSRAFastQ(FILE* fp);

/**
 * @param p1 first element for comparison, its type is SRAFastQ*
 * @param p2 sedond element for comparison, its type is SRAFastQ*
 * @return result of comparison
 */
int sra_read_comparison(const void* fq1, const void* fq2);

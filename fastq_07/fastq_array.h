#define INITIAL_FQ_ARRAY_CAPACITY 2

typedef struct {
    void** buf;
    int capacity;
    int length;
} FastQArray;

void FastQArray_init(FastQArray*);
void FastQArray_ensure(FastQArray*, int index);
void FastQArray_push_back(FastQArray*, void*);
int FastQArray_parse_index(char*);
void FastQArray_finish(FastQArray*);

// int sra_read_comparison(const void*, const void*);

void FastQArray_sort(FastQArray*, int(*usr_cmp_func)(const void*, const void*));
void bubble_sort(FastQArray*, int(*usr_cmp_func)(const void*, const void*));

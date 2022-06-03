#define INITIAL_FQ_ARRAY_CAPACITY 2

typedef struct {
    void** buf;
    int capacity;
    int length;
} FastQArray;

/**
 * @param arr   Initialize the array arr
 */
void FastQArray_init(FastQArray* arr);
/**
 * @paramm arr Check capacity and extend it automatically.
 **/
void FastQArray_ensure(FastQArray* arr, int index);
/**
 * @paramm arr Add element into this arr
 * @param  p   Pointer to the element to be added
 **/
void FastQArray_push_back(FastQArray*, void* p);
/**
 * @param  p read ma,e
 * @return index number
 */
int FastQArray_parse_index(char* p);
/**
 * @paramm arr Pointer to the array that should be erased
 **/
void FastQArray_finish(FastQArray*);

/**
 * @param arr The array to be sorted
 * @param usr_cmp_func Pointer to a function used for comparison
 */
void FastQArray_sort(FastQArray* arr, int(*usr_cmp_func)(const void*, const void*));

/**
 * @param arr The array to be sorted
 * @param usr_cmp_func Pointer to a function used for comparison
 */
void bubble_sort(FastQArray* arr, int(*usr_cmp_func)(const void*, const void*));

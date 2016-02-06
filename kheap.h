



#define HEAP_INDEX_SIZE   0x2000
#define HEAP_MAGIC        0xDEADBABE
#define HEAP_MIN_SIZE     0x0 // not used in first version

#include "common.h"
#include "ordered_list.h"

typedef struct
{
	uint8_t is_hole;
	uint32_t magic;
	uint32_t size;
} heap_header_t;

typedef struct
{
	heap_header_t *header_ptr;
	uint32_t magic;
} heap_footer_t;

typedef struct
{
	ordered_list_t index;
	uint32_t start_address;
	uint32_t end_address;
	uint8_t kernel_mode;
	uint8_t read_only;
} heap_t;

void *HEAPalloc( uint32_t size, uint8_t page_align, heap_t *heap);
void HEAPfree(void* ptr, heap_t *heap);

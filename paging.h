/** memory.h:
	* Defines the strucures for memory management, including paging
	*/

#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

#include "common.h"

typedef struct page
{
	uint32_t present			: 1; // page present in memory
	uint32_t rw						: 1; // page is writable
	uint32_t user					: 1; // page is user mode
	uint32_t referenced		: 1; // page has been referenced
	uint32_t dirty				: 1; // page has changed (written to)
	uint32_t unused				: 7; // unused but i dont get the order CHECK
	uint32_t frame				: 20;// frame address (4-aligned)
} page_t;


typedef struct __attribute__ ((packed)){
    int pagetable;
    int page;
}pageinfo, *ppageinfo;

uint32_t mm_allocphyspage();

void initpaging();

void switch_page_directory(uint32_t *new);

void page_fault(uint32_t err);

#endif

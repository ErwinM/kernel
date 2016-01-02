/** memory.h:
	* Defines the strucures for memory management, including paging
	*/

#ifndef INCLUDE_MEMORY_H
#define INCLUDE_MEMORY_H

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

typedef struct page_table
{
	page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
    /**
       Array of pointers to pagetables.
    **/
    page_table_t *tables[1024];
    /**
       Array of pointers to the pagetables above, but gives their *physical*
       location, for loading into the CR3 register.
    **/
    uint32_t tablesPhysical[1024];

    /**
       The physical address of tablesPhysical. This comes into play
       when we get our kernel heap allocated and the directory
       may be in a different location in virtual memory.
    **/
    uint32_t physicalAddr;
} page_directory_t;

/**
  Sets up the environment, page directories etc and
  enables paging.
**/
void initialise_paging();

/**
  Causes the specified page directory to be loaded into the
  CR3 register.
**/
void switch_page_directory(page_directory_t *new);

/**
  Retrieves a pointer to the page required.
  If make == 1, if the page-table in which this page should
  reside isn't created, create it!
**/
page_t *get_page(uint32_t address, int make, page_directory_t *dir);

/**
  Handler for page faults.
**/
void page_fault(regs_t regs);

#endif

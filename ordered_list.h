/** ordered_list.h -- Interface for creating, inserting and deleting
  *from ordered arrays.
  * Adapted from JamesM's kernel development tutorials.
	*/

#ifndef ORDERED_LIST_H
#define ORDERED_LIST_H

#include "common.h"

  // It can store anything that can be cast to a void* -- so a u32int, or any pointer.
typedef void* type_t;

	// A predicate should return nonzero if the first argument is less than the second. Else
  // it should return zero.
typedef int8_t (*lessthan_predicate_t)(type_t,type_t);


// The list itself
typedef struct
{
   type_t *list;
   uint32_t size;
   uint32_t max_size;
   lessthan_predicate_t less_than;
} ordered_list_t;

  // A standard less than predicate.
uint8_t standard_lessthan_predicate(type_t a, type_t b);

  // Create an ordered array.
ordered_list_t create_ordered_list(uint32_t max_size, lessthan_predicate_t less_than);
ordered_list_t place_ordered_list(void *addr, uint32_t max_size, lessthan_predicate_t less_than);

  // Destroy an ordered array.
void destroy_ordered_list(ordered_list_t *ordered_list);

  // Add an item into the array.
void insert_ordered_list(type_t item, ordered_list_t *ordered_list);

  // Lookup the item at index i.
type_t lookup_ordered_list(uint32_t i, ordered_list_t *ordered_list);

  // Deletes the item at location i from the array.
void remove_ordered_list(uint32_t i, ordered_list_t *ordered_list);

#endif // ORDERED_ARRAY_H

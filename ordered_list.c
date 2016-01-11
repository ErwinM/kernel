/** ordered_list.c:
  * Implements an ordered list which remains in a sorted state always
	*/

#include "common.h"
#include "ordered_list.h"


uint8_t standard_lessthan_predicate(type_t a, type_t b)
{
	return(a<b)?1:0;
}

ordered_list_t place_ordered_list(void *addr, uint32_t max_size, lessthan_predicate_t less_than)
{
   ordered_list_t to_ret;
   to_ret.list = (void*)addr;
   memset(to_ret.list, 0, max_size*sizeof(type_t));
   to_ret.size = 0;
   to_ret.max_size = max_size;
   to_ret.less_than = less_than;
   return to_ret;
}

void insert_ordered_list(void* item, ordered_list_t *ordered_list)
{
	ASSERT(ordered_list->less_than);
	uint32_t i=0;
	while (i < ordered_list->size && ordered_list->less_than(ordered_list->list[i], item))
	{
		i++;
	}
	if (i == ordered_list->size)
	{
		// just add it to the end of the list
		ordered_list->list[i] = item;
		ordered_list->size += 1;
		//fb_printf("got item: %h", item);
		//fb_printf("LOC: %h", &ordered_list->list[i]);
		//fb_printf("Value: %h", ordered_list->list[i]);
	} else {
		// insert the item at position i and shift all following items up once
		// first shift the rest up once
		uint32_t k;
		ordered_list->size += 1;
		for (k = ordered_list->size ; k > i ; k--)
		{
			ordered_list[k] = ordered_list[k-1];
		}
		ordered_list->list[i] = item;
	}
}

type_t lookup_ordered_list(uint32_t i, ordered_list_t *ordered_list)
{
	ASSERT(i<=ordered_list->size);
	//fb_printf("Lookup internal returning: %h", ordered_list->list[i]);
	return ordered_list->list[i];
}

void remove_ordered_list(uint32_t item, ordered_list_t *ordered_list)
{
	while (item < ordered_list->size)
   {
       ordered_list->list[item] = ordered_list->list[item+1];
       item++;
   }
   ordered_list->size--;
}

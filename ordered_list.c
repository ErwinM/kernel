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
	//fb_printf("Inserting...\n",item);
	ASSERT(ordered_list->less_than);
	uint32_t i=0;
	while (i < ordered_list->size && ordered_list->list[i] < item)
	{
		i++;
	}
	if (i == ordered_list->size)
	{
		//fb_write("INSERT: adding to the back");
		// just add it to the end of the list
		//ordered_list->list[ordered_list->size++];

		ordered_list->list[i] = item;
		ordered_list->size++;
		if ( item == 0xe08000 )
		{
			//fb_printf("Iterator: %d", i);
			//fb_printf("Size: %d", ordered_list->size);
			//fb_printf("index[i]: %h", ordered_list->list[i]);
		}
		//fb_printf("got item: %h", item);
		//fb_printf("Iteratior: %d", i);
		//fb_printf("Size: %d", ordered_list->size);
	} else {
		// insert the item at position i and shift all following items up once
		// first shift the rest up once
		fb_write("INSERT: inserting in front");
		uint32_t k;
		ordered_list->size++;
		//fb_printf("starting loop with i: %d", i);
		//fb_printf("and size: %d", ordered_list->size);
		for (k = (ordered_list->size-1) ; k > i ; k--)
		{
			ordered_list->list[k] = ordered_list->list[k-1];
			//fb_printf("inserting: %h", ordered_list->list[k-1]);
			//fb_printf("in spot: %d", k);
		}
		ordered_list->list[i] = item;
	}
	//fb_printf("exiting insert with size: %d", ordered_list->size);
}


type_t lookup_ordered_list(uint32_t i, ordered_list_t *ordered_list)
{
	ASSERT(i<=ordered_list->size);
	//fb_printf("Lookup internal returning: %h", ordered_list->list[i]);
	return ordered_list->list[i];
}

void remove_ordered_list(uint32_t item, ordered_list_t *ordered_list)
{
	//fb_write("Removing....\n");
	while (item < ordered_list->size)
   {
       ordered_list->list[item] = ordered_list->list[item+1];
       item++;
   }
   ordered_list->size--;
}

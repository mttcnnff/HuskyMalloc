#include <stdlib.h>
#include <stdio.h>
#include "freelist.h"

/*

typedef struct {
	size_t size;
	void* address;
	struct freeblock* next;
} freeblock;

typedef struct {
	size_t size;
	freeblock* head;
} freelist;

*/

freeblock* 
freeblock_make(size_t size, void* address)
{
	freeblock* new = malloc(sizeof(freeblock));
	new->size = size;
	new->address = address;
	new->next = NULL;
	return new;
}

void 
freeblock_free(freeblock* block)
{
	if (block->next)
	{
		freeblock_free(block->next);
	}
	free(block);
}

void 
freeblock_print(freeblock* block)
{
	if (block) {
		printf("[Address: %p, Size: %zu]\n", block->address, block->size);
	}
	return;
}

freelist* 
freelist_make()
{
	freelist* new = malloc(sizeof(freelist));
	new->size = 0;
	new->head = NULL;
	return new;
}

void 
freelist_free(freelist* list)
{
	if (list->head)
	{
		freeblock_free(list->head);
	}
	free(list);
}

void
freelist_insert(freelist* list, size_t size, void* address)
{
	if (list->head == NULL)
	{
		list->head = freeblock_make(size, address);
	}

	// freeblock* previous = list->;
	// while (current && current->address < address) {
	// 	//move to next
	// 	previous = current;
	// 	current = current->next;
	// }

	return;
}

void
freelist_print(freelist* list)
{
	printf("Size: %zu\n", list->size);

	freeblock* current = list->head;
	while (current) {
		freeblock_print(current);
		current = current->next;
	}
	return;
}

#ifndef FREELIST_H
#define FREELIST_H

// Free List Requirements:
// Singly Linked List
// Sorted by Address
// Insert into free list
// remove from free list
// Join two adjacent blocks

typedef struct freeblock {
	size_t size;
	void* address;
	struct freeblock* next;

} freeblock;

typedef struct {
	size_t size;
	struct freeblock* head;
} freelist;

freelist* freelist_make();
void freelist_insert(freelist* list, size_t size, void* address);
void* freelist_getblock(freelist* list, size_t size);
void freelist_free(freelist* list);
void freelist_print(freelist* list);

freeblock* freeblock_make(size_t size, void* address);
void freeblock_free(freeblock* block);
void freeblock_print(freeblock* block);


#endif

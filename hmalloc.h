#ifndef HMALLOC_H
#define HMALLOC_H

// Husky Malloc Interface
// cs3650 Starter Code

typedef struct hm_stats {
    long pages_mapped;
    long pages_unmapped;
    long chunks_allocated;
    long chunks_freed;
    long free_length;
} hm_stats;

typedef struct freeblock {
	size_t size;
	struct freeblock* next;
} freeblock;

hm_stats* hgetstats();
void hprintstats();

void* hmalloc(size_t size);
void hfree(void* item);
void freelist_print();
int freelist_sorted();
void freelist_checkadj();

#endif

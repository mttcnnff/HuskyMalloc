
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

#include "hmalloc.h"

/*
  typedef struct hm_stats {
  long pages_mapped;
  long pages_unmapped;
  long chunks_allocated;
  long chunks_freed;
  long free_length;
  } hm_stats;
*/

const size_t PAGE_SIZE = 4096;
static hm_stats stats; // This initializes the stats to 0.
static freeblock* head = NULL;

static
freeblock*
freeblock_make(void* address)
{

  size_t size = *(size_t*)address;
  if (size > sizeof(freeblock)) 
  {
    freeblock* new = (freeblock*)address;
    new->size = size;
    new->next = NULL;
    return new;
  }
  printf("Not enough memory for freeblock!\n");
  return NULL;
}

static
freeblock* 
freelist_remove(freeblock** head, freeblock* block)
{
  freeblock* current = *head;
  freeblock* previous = NULL;

  while (current && current != block)
  {
    previous = current;
    current = current->next;
  }

  if (current) {
    if (previous) {
      previous->next = current->next;
    } else {
      //removing head
      *head = current->next;
    }
    current->next = NULL;
    return current;
  } else {
    return NULL;
  }
}

static
int 
freelist_getsize(freeblock* head)
{
  int size = 0;
  freeblock* current = head;
  while (current) {
    current = current->next;
    size++;
  }
  return size;
}

static
void 
freeblock_print(freeblock* block)
{
  if (block) {
    printf("[Address: %p, Size: %zu]\n", block, block->size);
  }
  return;
}

void
freelist_print()
{
  int size = freelist_getsize(head);
  printf("Freelist Size: %d\n", size);
  freeblock* current = head;
  while (current) {
    freeblock_print(current);
    current = current->next;
  }
  return;
}

int 
freelist_sorted()
{
  freeblock* current = head;
  while (current && current->next) {
    if (current > current->next) {
      return 0;
    }
    current = current->next;
  }
  return 1;
}

void 
freelist_checkadj()
{
  freeblock* current = head;
  while (current && current->next) {
    if ((void*)current + current->size >= (void*)(current->next)) {
      printf("\n\n\n=========== FOUND OVERLAP ============\n\n\n");
      exit(1);
    }
    current = current->next;
  }
}

// static
// void 
// freelist_insert(freeblock** head, void* addition)
// {

//   freeblock* current = *head;
//   freeblock* previous = NULL;
//   freeblock* newblock = NULL;

//   while (current && (void*)current < addition)
//   {
//     previous = current;
//     current = current->next;
//   }

//   void* previous_memory_end = (void*)previous ? (void*)previous + previous->size : NULL;
//   void* newblock_memory_begin = addition;
//   void* newblock_memory_end = addition + *(size_t*)addition;
//   void* current_memory_begin = (void*)current ? (void*)current : NULL;

//   printf("Prev End: \t%p\nNew Begin: \t%p\nNew End: \t%p\nNext Begin: \t%p\n", previous_memory_end, newblock_memory_begin, newblock_memory_end, current_memory_begin);

//   if (previous_memory_end != newblock_memory_begin && newblock_memory_end != current_memory_begin) {
//     newblock = freeblock_make(addition);
//     if (previous)
//     {
//       previous->next = newblock;
//     } else {
//       *head = newblock;
//     }
//     newblock->next = current;
//   } else {
//     if (previous_memory_end == newblock_memory_begin) {
//       size_t prev_new_size = previous->size + *(size_t*)addition;
//       memcpy(previous, &prev_new_size, sizeof(size_t));
//       newblock = previous;
//     }

//     if (newblock_memory_end == current_memory_begin) {
//       printf("Expand next.\n");
//       if (newblock) {
//         //expand newblock
//         size_t newblock_new_size = newblock->size + current->size;
//         memcpy(newblock, &newblock_new_size, sizeof(size_t));
//         freelist_remove(head, current);
//       } else {
//         //expand current backward
//         size_t current_new_size = current->size + *(size_t*)addition;
//         memcpy(current, &current_new_size, sizeof(size_t));
//       }
//     }
//   }
// }

static
void
freeblock_expand_back(freeblock** block, size_t size) {
  //freeblock* current = *block;
  size_t new_size = (*block)->size + size;
  memcpy((void*)*block - size, &new_size, sizeof(size_t));
  *block = freeblock_make((void*)*block - size);
}

static
void 
freelist_insert(freeblock** head, void* addition)
{

  printf("Inserting: %p -> %p, Size: %zu\n", addition, addition + *(size_t*)addition, *(size_t*)addition);

  freeblock* current = *head;
  freeblock* previous = NULL;
  freeblock* newblock = NULL;

  while (current && (void*)current < addition)
  {
    previous = current;
    current = current->next;
  }

  void* previous_memory_end = (void*)previous ? (void*)previous + previous->size : NULL;
  void* newblock_memory_begin = addition;
  void* newblock_memory_end = addition + *(size_t*)addition;
  void* current_memory_begin = (void*)current ? (void*)current : NULL;

  printf("Prev End: \t%p\nNew Begin: \t%p\nNew End: \t%p\nNext Begin: \t%p\n", previous_memory_end, newblock_memory_begin, newblock_memory_end, current_memory_begin);

  if (previous_memory_end != newblock_memory_begin && newblock_memory_end != current_memory_begin) {
    newblock = freeblock_make(addition);
    if (previous)
    {
      previous->next = newblock;
    } else {
      *head = newblock;
    }
    newblock->next = current;
  } else {
    if (previous_memory_end == newblock_memory_begin) {
      printf("Expand previous.\n");
      size_t prev_new_size = previous->size + *(size_t*)addition;
      previous->size = prev_new_size;
      newblock = previous;
    } 

    if (newblock_memory_end == current_memory_begin) {
      printf("Expand next.\n");
      if (newblock) {
        //expand newblock
        size_t newblock_new_size = newblock->size + current->size;
        newblock->size = newblock_new_size;
        freelist_remove(head, current);
      } else {
        freeblock_expand_back(&current, *(size_t*)addition);
        //expand current backward
        // size_t current_new_size = current->size + *(size_t*)addition;
        // memcpy(current, &current_new_size, sizeof(size_t));
      }
    }
  }

  freelist_print();
  freelist_checkadj();
}

static
void* 
freelist_getblock(freeblock** head, size_t size) 
{
  freeblock* current = *head;
  while (current && current->size < size) {
    current = current->next;
  }

  if (current) {
    
    current = freelist_remove(head, current);
    void* memory = (void*)current;
    size_t leftover = current->size - size;
    if (leftover > sizeof(freeblock))
    {
      memcpy(memory, &leftover, sizeof(size_t));
      freelist_insert(head, memory);
    } else {
      size+=leftover;
      leftover = 0;
    }

    // size_t leftover = PAGE_SIZE - size;
    // if (leftover > sizeof(freeblock)) {
    //   //insert leftover into freelist
    //   memcpy(memory, &leftover, sizeof(size_t));
    //   freelist_insert(&head, memory);
    // } else {
    //   size+=leftover;
    //   leftover = 0;
    // }
    // memcpy(memory + leftover, &size, sizeof(size_t));
    // return memory + leftover + sizeof(size_t);

    printf("Size of block being returned: %zu\n", size);
    memcpy(memory + leftover, &size, sizeof(size_t));
    return memory + leftover;
  } else {
    
    return 0;
  }
}

long
free_list_length()
{
    return freelist_getsize(head);
}

hm_stats*
hgetstats()
{
    stats.free_length = free_list_length();
    return &stats;
}

void
hprintstats()
{
    stats.free_length = free_list_length();
    fprintf(stderr, "\n== husky malloc stats ==\n");
    fprintf(stderr, "Mapped:   %ld\n", stats.pages_mapped);
    fprintf(stderr, "Unmapped: %ld\n", stats.pages_unmapped);
    fprintf(stderr, "Allocs:   %ld\n", stats.chunks_allocated);
    fprintf(stderr, "Frees:    %ld\n", stats.chunks_freed);
    fprintf(stderr, "Freelen:  %ld\n", stats.free_length);
}


static
size_t
div_up(size_t xx, size_t yy)
{
    // This is useful to calculate # of pages
    // for large allocations.
    size_t zz = xx / yy;

    if (zz * yy == xx) {
        return zz;
    }
    else {
        return zz + 1;
    }
}


// void*
// hmalloc(size_t size)
// {
//     void* memory;
//     stats.chunks_allocated += 1;
//     size += sizeof(size_t);
//     printf("Hmallocing: %zu from\n", size);
//     freelist_print();

//     if (size < PAGE_SIZE) {
//       memory = freelist_getblock(&head, size);

//       if(memory) {
//         return memory + sizeof(size_t);
//       } else {
//         printf("\n========= NOT ENOUGH MEMORY ON FREELIST! =========\n\n");
//         stats.pages_mapped += 1;
//         memory = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
//         size_t leftover = PAGE_SIZE - size;
//         if (leftover > sizeof(freeblock)) {
//           //insert leftover into freelist
//           memcpy(memory + size, &leftover, sizeof(size_t));
//           freelist_insert(&head, memory + size);
//         } else {
//           size+=leftover;
//         }
//         memcpy(memory, &size, sizeof(size_t));
//         return memory + sizeof(size_t);
//       }

//     } else {
//       int pages = div_up(size, PAGE_SIZE);
//       printf("Making %d pages.\n", pages);
//       size_t new_size = pages*PAGE_SIZE;
//       stats.pages_mapped += pages;
//       memory = mmap(0, new_size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
//       memcpy(memory, &new_size, sizeof(size_t));
//       return memory + sizeof(size_t);
//     }
// }

void*
hmalloc(size_t size)
{
    void* memory;
    stats.chunks_allocated += 1;
    size += sizeof(freeblock);
    printf("Hmallocing: %zu from\n", size);
    //freelist_print();

    if (size < PAGE_SIZE) {
      memory = freelist_getblock(&head, size);

      if(memory) {
        return memory + sizeof(size_t);
      } else {
        printf("\n========= NOT ENOUGH MEMORY ON FREELIST! =========\n\n");
        stats.pages_mapped += 1;
        memory = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
        size_t leftover = PAGE_SIZE - size;
        if (leftover > sizeof(freeblock)) {
          //insert leftover into freelist
          memcpy(memory, &leftover, sizeof(size_t));
          freelist_insert(&head, memory);
        } else {
          size+=leftover;
          leftover = 0;
        }
        memcpy(memory + leftover, &size, sizeof(size_t));
        return memory + leftover + sizeof(size_t);
      }

    } else {
      int pages = div_up(size, PAGE_SIZE);
      printf("Making %d pages.\n", pages);
      size_t new_size = pages*PAGE_SIZE;
      stats.pages_mapped += pages;
      memory = mmap(0, new_size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
      memcpy(memory, &new_size, sizeof(size_t));
      return memory + sizeof(size_t);
    }
}

void
hfree(void* item)
{
    stats.chunks_freed += 1;

    void* chunk_to_free = item - sizeof(size_t);

    size_t size = *(size_t*)chunk_to_free;
    //freelist_print();
    int pages = div_up(size, PAGE_SIZE);

    if (size < PAGE_SIZE) {
      //freelist_insert(&head, chunk_to_free);
      if (size >= sizeof(freeblock)) {
        freelist_insert(&head, chunk_to_free);
      } 
      
    } else {
      stats.pages_unmapped += pages;
      munmap(chunk_to_free, size);
    }

    //freelist_print();

    // TODO: Actually free the item.
}

size_t 
memory_getsize(void* memory)
{
  return *(size_t*)(memory - sizeof(size_t)) - sizeof(size_t);
}

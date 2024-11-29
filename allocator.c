#define _GNU_SOURCE

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// The minimum size returned by malloc
#define MIN_MALLOC_SIZE 16
#define MAGIC_NUMBER 102958

// Round a value x up to the next multiple of y
// #define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

// The size of a single page of memory, in bytes
#define PAGE_SIZE 0x1000

// creaeting the page header that goes at the beginning of our free list
typedef struct page_header {
  int magic_number;
  size_t size_of_blocks;
} page_header_t;

page_header_t* init_header(page_header_t* header, int size) {
  header->magic_number = MAGIC_NUMBER;
  header->size_of_blocks = (size_t)size;
  return header;
}

// free list struct that goes next
//  defining our freelists
typedef struct free_list {
  struct free_list* next;
} free_list_t;

// initialize the get_list function
free_list_t* get_list(size_t size);
// initialize the put_list function
void put_list(free_list_t* freelist, size_t size);

// setting all of our free_lists to NULL
free_list_t* free_lists[8] = {NULL};

// initializing our round-up function
size_t round_up(size_t size);

// A utility logging function that definitely does not call malloc or free
void log_message(char* message);
/**
 * char buffer[128];
 * snprintf(buffer, 128, "malloc of size %zu\n", size);
 * log_message(buffer);
 */

/**
 * Allocate space on the heap.
 * \param size  The minimium number of bytes that must be allocated
 * \returns     A pointer to the beginning of the allocated space.
 *              This function may return NULL when an error occurs.
 */
void* xxmalloc(size_t size) {
  // if the size is too big (2048 for now), we just rely on mmap
  if (size > 2048) {
    void* p =
        mmap(NULL, round_up(size), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    // Check for errors
    if (p == MAP_FAILED) {
      log_message("mmap failed! Giving up.\n");
      exit(2);
    }
    return p;
  }

  // Round the size up to the next power of 2, minimum 16, max 2048.
  size = round_up(size);

  // 1) check if a block is available in a free list

  // 1.1 select the list of appropriate size-the get list function gives us the freelist of the
  // correctly sized blocks
  free_list_t* freelist = get_list(size);

  // 1.2 check if freelist is uninitialized or empty

  if (freelist == NULL) {
    // 2) if no free memory in freelist, use mmap to request new memory
    // 2.1) use mmap of the size of the page
    void* p = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    // Check for errors
    if (p == MAP_FAILED) {
      log_message("mmap failed! Giving up.\n");
      exit(2);
    }

    // 2.2) make first block into header (header pointer is = to p)
    page_header_t* header = p;
    header = init_header(header, size);

    // convert p into intptr type to do math on pointers
    intptr_t p_num = (intptr_t)p;
    // make free list point to the first block past the header
    freelist = (free_list_t*)(p_num + size);

    // calculate the number of blocks using block size
    int number_of_blocks = PAGE_SIZE / size;

    for (int i = 2; i < number_of_blocks; i++) {
      // allocating blocks for one page
      // convert freelist into intptr type to do math on pointers
      intptr_t freelist_num = (intptr_t)freelist;
      freelist->next = (free_list_t*)(freelist_num + size);
      freelist = freelist->next;
    }
    // end the free list
    freelist->next = NULL;

    // get back the beginning of freelist
    freelist = (free_list_t*)(p_num + size);
  }

  // 3.1 list has a block of memory, get the block, modify the list
  // record the block to be returned
  void* block_return = freelist;
  // regroup free list
  freelist = freelist->next;

  put_list(freelist, size);

  // 3.2 return the block
  return block_return;
}

/**
 * Free space occupied by a heap object.
 * \param ptr   A pointer somewhere inside the object that is being freed
 */
void xxfree(void* ptr) {
  // Don't free NULL!
  if (ptr == NULL) return;

  // TODO: Complete this function

  // Treat the freed pointer as an integer
  intptr_t free_address = (intptr_t)ptr;

  // Round down to the beginning of a page
  intptr_t page_start = free_address - (free_address % PAGE_SIZE);

  // Cast the page start address to a header struct
  page_header_t* header = (page_header_t*)page_start;

  if (header->magic_number != MAGIC_NUMBER) {
    return;
  }

  size_t size = header->size_of_blocks;

  free_list_t* freelist = get_list(size);

  free_list_t* newlist = (free_list_t*)ptr;

  newlist->next = freelist;

  put_list(newlist, size);
  return;
}

/**
 * Get the available size of an allocated object. This function should return the amount of space
 * that was actually allocated by malloc, not the amount that was requested.
 * \param ptr   A pointer somewhere inside the allocated object
 * \returns     The number of bytes available for use in this object
 */
// LD_PRELOAD=./myallocator.so ./test/malloc-test
size_t xxmalloc_usable_size(void* ptr) {
  // If ptr is NULL always return zero
  if (ptr == NULL) {
    return 0;
  }

  // Treat the freed pointer as an integer
  intptr_t free_address = (intptr_t)ptr;

  // Round down to the beginning of a page
  intptr_t page_start = free_address - (free_address % PAGE_SIZE);

  // Cast the page start address to a header struct
  page_header_t* header = (page_header_t*)page_start;

  // if not one of our pages, do nothing, return 0
  if (header->magic_number != MAGIC_NUMBER) {
    return 0;
  }

  int size = (int)header->size_of_blocks;
  return size;
}

/**
 * Print a message directly to standard error without invoking malloc or free.
 * \param message   A null-terminated string that contains the message to be printed
 */
void log_message(char* message) {
  // Get the message length
  size_t len = 0;
  while (message[len] != '\0') {
    len++;
  }

  // Write the message
  if (write(STDERR_FILENO, message, len) != len) {
    // Write failed. Try to write an error message, then exit
    char fail_msg[] = "logging failed\n";
    write(STDERR_FILENO, fail_msg, sizeof(fail_msg));
    exit(2);
  }
}

// get_list(int size): Finds the list of appropriate block sizes
/**Input:
 *    size: the size of blocks the list must contain
 * Output:
 *    free_list_t*: a pointer to the list of correct block sizes
 */
free_list_t* get_list(size_t size) {
  switch (size) {
    case 16:
      return free_lists[0];
    case 32:
      return free_lists[1];
    case 64:
      return free_lists[2];
    case 128:
      return free_lists[3];
    case 256:
      return free_lists[4];
    case 512:
      return free_lists[5];
    case 1024:
      return free_lists[6];
    case 2048:
      return free_lists[7];
  }
  log_message("SHOULD NEVER PRINT THIS\n");
  return NULL;
}

void put_list(free_list_t* freelist, size_t size) {
  switch (size) {
    case 16:
      free_lists[0] = freelist;
      break;
    case 32:
      free_lists[1] = freelist;
      break;
    case 64:
      free_lists[2] = freelist;
      break;
    case 128:
      free_lists[3] = freelist;
      break;
    case 256:
      free_lists[4] = freelist;
      break;
    case 512:
      free_lists[5] = freelist;
      break;
    case 1024:
      free_lists[6] = freelist;
      break;
    case 2048:
      free_lists[7] = freelist;
      break;
  }
  return;
}

// rounds up on base 2
size_t round_up(size_t size) {
  size_t return_size = 16;
  while (return_size < size) {
    return_size *= 2;
  }
  return return_size;
}
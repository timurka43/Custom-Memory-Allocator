# Custom Memory Allocator 

## Overview
This project shows an implementation of a custom memory allocator that closely replicates the functionality of `malloc()` and `free()` in C. It was pair-programmed with Budhil Thijm.


## Key Features
- **Custom malloc() and free():** Implements memory allocation and deallocation similar to `malloc()` and `free()`, using custom logic and pointer manipulation.
- **Page Management:** Uses `mmap()` to allocate "pages" of memory, which are linked together to form a pool of memory.
- **Efficient Pointer Arithmetic:** We performed extensive pointer manipulation (such as pointer subtraction and modulus operations) to implement the allocator.
- **Memory Pages:** Memory chunks are allocated in sizes from 16KB to 2048KB, with each individual page being 4096 bytes.
- **Linked List of Pages:** Memory is managed as a linked list of pages, with each list containing multiple chunks of memory that can be allocated dynamically.


## Contributions
### Personal Contributions:
  * I implemented the core memory allocator logic with fixed-size blocks of memory
  * I fine-tuned the implementation of pointer arithmetic, including addition and subtraction on pointers, as well as modulus operations
        * Since malloc() and free() calls aren't available to us in this project, I manipulated pointers mathematically to access free blocks available for allocations
        * I also used modulus operations on pointers to access the first memory block in the current page to access its magic number, thus figuring out whether the memory block was allocated using the custom malloc() function.

### Budhil Thijm:
  * Collaborated in the implementation of the allocator
  * Implemented size-specific linked lists to store the available blocks of memory of a given size


## Acknowledgments
I would like to thank professor Curtsinger for providing the initial framework and for guiding us through the process of building a custom memory allocator from scratch, as well as allowing me to display this project online.

## License
This project is licensed under the MIT License

# Memory_allocator
Simple memory allocator in C using a data structure called "Segregated free lists".
This particular data structure is implemented using an array of doubly linked lists. 
Usage:
* INIT_HEAP <start_address> <number_of_lists_in_the_array> <bytes_per_list>
* MALLOC <bytes>
* FREE <address>
* READ <address> <bytes>
* WRITE <address> <data> <bytes>
* DUMP_MEMORY
* DESTROY_HEAP
Error messages:
* OUT_OF_MEMORY : when allocating memory beyond the heap's capacity
* INVALID_FREE: when freeing unallocated memory space
* SEGMENTATION_FAULT: when reading from / writing at an invalid address
  

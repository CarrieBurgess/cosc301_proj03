/*

Title: Project 03
Purpose: to recreate a method to malloc goodness and find a way to let free the evils of the world
(make a malloc and free)
Masterminds: Shreeya Khadka, the Robot from Pluto, and Carrie Burgess, an evil world ruler
Date of world domination (when due): 4 November 2013

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// function declarations
void *malloc(size_t);
void free(void *);
void dump_memory_map(void);


const int HEAPSIZE = (1*1024*1024); // 1 MB
const int MINIMUM_ALLOC = sizeof(int) * 2;

// global file-scope variables for keeping track
// of the beginning of the heap.
void *heap_begin = NULL;
void *first_free = NULL;

void write_header(void *p, int size, int offset) { 
	*((int*)(p)) = size;
	*((int*)(p)+1)=offset;
}

void read_header(void*p, int * size, int * offset) { 
//we are assuming size and offset will be declared on the heap, and that the pointer to the chunk
//starts before the header
	*size = *((int *)(p));
	*offset = *((int *)(p) + 1);
}
	
void *malloc(size_t request_size) {

    // if heap_begin is NULL, then this must be the first
    // time that malloc has been called.  ask for a new
    // heap segment from the OS using mmap and initialize
    // the heap begin pointer.
   if(request_size==0) {
    	printf("Please enter a valid size.\n");
    	return NULL;
    }
    if (!heap_begin) {
        heap_begin = mmap(NULL, HEAPSIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
		
        atexit(dump_memory_map);
        write_header(heap_begin, (1024*1024), 0);
        first_free = heap_begin; //this points to first free chunk, BEFORE header
    }
    request_size = request_size+8; //add 8 bits
    int i = 0;
	int pow = 1;
    while(pow<request_size) { 
		pow = pow*2;
    	i++;
    } 
    request_size = pow;
	printf("size %d\n",(int) request_size);
	
	
   return NULL; //CHANGE THIS!!  Need to return pointer to allocated space (after 8 bits)
}

void free(void *memory_block) {
	
}



void dump_memory_map(void) {
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	int size;
	int offset = 0; //difference between chunks
	void * pointer = first_free;
	if(pointer != heap_begin) { //deals with if malloced memory at beginning
		int size = (char*)first_free - (char*)heap_begin;
		printf("Block size: %d bits.  Segment is allocated.\n", size);
	}
	read_header(pointer, &size, &offset);
	printf("Block size: %d bits.  Offset: %d bits.  Segment is free.\n", size, offset);
	while(offset != 0) {
		if(offset>size) { //for segments allocated b/w free chunks
			size = offset-size;
			printf("Block size: %d bits.  Segment is allocated.\n", size);
		}
		pointer = pointer + offset; //units should work...
		read_header(pointer, &size, &offset);	
		printf("Block size: %d bits.  Offset: %d bits.  Segment is free.\n", size, offset);
	}
	if(offset>size) { //if there is one last allocated chunk after last free chunk
		size = offset-size;
		printf("Block size: %d bits.  Segment is allocated.\n", size);
	}
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");	
}



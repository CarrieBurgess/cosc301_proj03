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

static void write_header(void *p, int size, int offset) { 
	*((int*)(p)) = size;
	*(((int*)p)+1)=offset;
}

static void read_header(void *p, int *size, int *offset) { 
//we are assuming size and offset will be declared on the heap, and that the pointer to the chunk
//starts before the header
	*size = *((int *)p);
	*offset = *(((int *)p) + 1);
}
	
static void *malloc_rec(int size, void *free_list) { 
	//recursively attempt to malloc free space over the free list 
	//if successful malloc address of malloced space else return NULL
	if (free_list == NULL) return NULL; //base
	int free_size = 0; 
	int offset = 0; 
	read_header(free_list, &free_size, &offset);
	if (free_size==size) { //size of free chunk = size of chunk that needs to be allocated --> allocated! get rid of next offset
			if (free_list==first_free) {//have to allocate on 1st block--happens only at the beginning
				int first_offset;
				int first_size;
				read_header(first_free, &first_size, &first_offset);
				if (first_offset!=0) { //need to update first_free
					first_free = first_free + first_offset;
					free_list = first_free;
				}
			}
			//else called by its recursive mates 
			return free_list; 
		}
	if (free_size<size) { //not enough try to allocate in next free element
		if (offset == 0) return NULL; //no next free blocks
		void * next = malloc_rec(size, (free_list + offset)); //check for match in next free_list element
		if (next==NULL) return NULL; //no match found
		else { //match found remove it from free_list
			int next_size;
			int next_offset;
			read_header(next, &next_size, &next_offset);
			write_header(free_list, free_size, offset + next_offset); //update free list
			write_header(next, next_size, 0); //set offset of allocated stuff to 0
			return next; //returns a non null address
		}
	}	
	else { //free_size>size :: break it up!
		free_size = free_size/2;
		write_header(free_list, free_size, free_size);
		if (offset>0) write_header(free_list+free_size , free_size, offset-free_size);
		else write_header(free_list+free_size , free_size, 0);
		return malloc_rec(size, free_list); //perform malloc_rec on new free_list
	}		
}

void *malloc(size_t request_size) {
    // if heap_begin is NULL, then this must be the first
    // time that malloc has been called.  ask for a new
    // heap segment from the OS using mmap and initialize
    // the heap begin pointer.
   if((request_size<MINIMUM_ALLOC)||(request_size>HEAPSIZE)) { //cannot be allocated
    	return NULL;
    }
    if (!heap_begin) { //initialization 
        heap_begin = mmap(NULL, HEAPSIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
		write_header(heap_begin, HEAPSIZE, 0);
        first_free = heap_begin; //this points to first free chunk, BEFORE header
        atexit(dump_memory_map);
    }
    request_size = request_size+8; //add 8 bytes
    int i = 0;
	int pow = 1;
    while(pow<request_size) { 
		pow = pow*2;
    	i++;
    } 
    request_size = pow;
	void *mallocked = malloc_rec((int) request_size, first_free);
	if (mallocked == NULL) return NULL;
   	return (mallocked+8); //else send mallocked+8 bytes 
}

void free(void *memory_block) {
	if (memory_block==NULL) return;
//find the matching block in heap
	int size;
	int offset;
	void *elem = heap_begin;
	int position = 0; 
	int total = 0;
	while(total<HEAPSIZE) {
		read_header(elem, &size, &offset);
		if (elem==memory_block) {
			position = total;
			break;
		}
		elem = elem + size;
		total = total + size;
	}
	if (!position) return; //no match else match is (heap_begin+position)
//next update the free list
	read_header(first_free, &size, &offset);
	int displacement = (char*)first_free - (char*)memory_block;
	if (displacement==0) return; //already free
	if (displacement>0) {//have to update first_free 
		*(((int *)memory_block)+1) = displacement; //update offset of memory block
			first_free = memory_block;
			return;   
	}
//have to go to right of first_free
	if (offset == 0) {//first=last free fix to end
		displacement = (-1)*displacement;
		write_header(first_free, size, displacement);
		*(((int *)memory_block)+1) = 0; //update offset of memory block
		return;
	}
	void *free_list = first_free;
	void *old_free = NULL;
	int change = 0;
	while (offset!=0) { 
		old_free = free_list;
		free_list = free_list + offset;
		read_header(free_list, &size, &offset);
		change = (char*)memory_block - (char*)free_list;
		if (change==0) return; //block free
		if (change<0) break; //block to the left of free_list, right of old_free		 
	}
	int old_offset = *(((int *)old_free)+1); //offset of old_free->distance to free_list
	*(((int *)old_free)+1) = change; //offset between old_free and block
	*(((int *)memory_block)+1) = old_offset - change; //offset between block and free_list		
}



void dump_memory_map(void) {
	printf("\n~~~~~~~~~~~~~~~~~~~~~~~Memory Dump~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
	int size;
	int offset;
	void *free_list = first_free;
	void *elem = heap_begin;
	int total = 0;
	while (total<HEAPSIZE) {
		read_header(elem, &size, &offset);
		if (elem==free_list) {//element is free
			printf("Block size: %d bits. Segment is free. Total offset: %d\n", size, total);
			free_list = free_list + offset; //next free is offset away from it
		}
		else {//element is allocated
			printf("Block size: %d bits. Segment is allocated. Total offset: %d\n", size,total);
		}
		total = total + size; //offset of the next = current(offset + size)
		elem = elem + size;
	}
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");	
}



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

//Carrie
static void write_header(void *p, int size, int offset) { 
	*((int*)(p)) = size;
	*(((int*)p)+1)=offset;
}

//Carrie
static void read_header(void *p, int *size, int *offset) { 
//we are assuming size and offset will be declared on the heap, and that the pointer to the chunk
//starts before the header
	*size = *((int *)p);
	*offset = *(((int *)p) + 1);
}
	
//Shreeya and ever so minorly Carrie
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
				//C: isn't this redundent? Exact same thing for last read_header
				//that was used to enter this if statement
					first_free = first_free + first_offset;
					//free_list = first_free;
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


//Shreeya and Carrie
void *malloc(size_t request_size) {
    // if heap_begin is NULL, then this must be the first
    // time that malloc has been called.  ask for a new
    // heap segment from the OS using mmap and initialize
    // the heap begin pointer.
    printf("entering malloc\n");
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

//Carrie
int buddy_loc(int block_size, int total) { //finds if buddy of block to be free is before or after block.  block_size should be size of memory_block
	int ans = (total/block_size)%2;
	if(ans==0) { //if even, its the first buddy
		return 0;
	}
	return 1; //if odd, its the second buddy
}

//Carrie
void coalesce_rec() {
	//recursively goes through and coalesces things until nothing else can be coalesced
	void * prev_check;
	void * check = first_free;
	int size;
	int offset;
	read_header(check, &size, &offset);
	int loc;
	int total;
	int change = 0;
	while(total<HEAPSIZE && offset!=0) {
		prev_check = check;
		check = check + offset;
		total = check - heap_begin;
		read_header(check, &size, &offset);
		loc = buddy_loc(size, total);
		if(loc!=0) { //second buddy
			if((check-size)==prev_check) { //if two buddies are free
				int new_offset;
				if(offset!=0) {
					new_offset = offset + size; //offset of prev_check was
					//size, so getting total offset
				}
				else {
					new_offset = 0;
				}
				check = prev_check;
				write_header(check, size*2, new_offset);
				change = 1;
				//as the two buddies had same size, new size is just size*2
			}
		}
	}
	if(change==1) {
		coalesce_rec(); //if there were changes, possibility that more opportunities
		//to coalesce appeared
	}
}


//Shreeya and Carrie
void free(void *memory_block) {
	if (memory_block==NULL) return;
//find the matching block in heap
	int size;
	int offset;
	void *elem = heap_begin;
	int position = -1; 
	int total = 0;
	memory_block = memory_block -8; //block returned points to after header
	while(total<HEAPSIZE) { //find the matching block
		read_header(elem, &size, &offset);
		//printf("Elem: %p\n", elem);
		//printf("memory_block: %p\n", memory_block);
		if (elem==memory_block) {
			position = total;
			break;
		}
		elem = elem + size;
		total = total + size;
	}
//	printf("entering free. memory map\n");
//	dump_memory_map();
	if (position==-1) { //no match else match is (heap_begin+position)
//next update the free list
		printf("There was no matching element found.\n");
		//printf("location a.  memory map: \n");
		//dump_memory_map();
		return;
	}	
	read_header(first_free, &size, &offset);
	//int displacement = (char*)first_free - (char*)memory_block;
	//C: I don't quite understand this line.
	int displacement = (char*)memory_block - (char*)first_free; //how far address is from
	//printf("First free: %p\n", first_free);
	//printf("memory_block: %p\n", memory_block);
	//1st free element
	//C: isn't this the same as total?
	//printf("Displacement: %d\n", displacement);
	if (displacement==0) { //base case: already free
		printf("The block has already been freed.\n");
		//printf("location b: memory map: \n");
		//dump_memory_map();
		return;
	}
	if (displacement<0) {//base case: have to update first_free if first_free is after memory_block
		*(((int *)memory_block)+1) = (-1)*displacement; //update offset of memory block
		first_free = memory_block;
		//printf("location c: memory map: \n");
		//dump_memory_map();
		coalesce_rec();
		return;   
	}
//have to go to right of first_free
	if (offset == 0) {//base case: first=last free fix to end --> first_free is the only
	//free, so new free unit (after first_free) will become last free block
		displacement = (-1)*displacement;
		write_header(first_free, size, displacement);
		*(((int *)memory_block)+1) = 0; //update offset of memory block
		//printf("location d: memory map: \n");
		//dump_memory_map();
		coalesce_rec();
		return;
	}
	void *free_list = first_free;
	void *old_free = NULL;
	int change = 0;
	while (offset!=0) { //if none of the base cases, then somewhere in middle
		old_free = free_list;
		free_list = free_list + offset;
		read_header(free_list, &size, &offset);
		change = (char*)memory_block - (char*)free_list;
		//printf("location e: memory map: \n");
		//dump_memory_map();
		if (change==0) return; //block free
		if (change<0) break; //block to the left of free_list, right of old_free		 
	}
	int old_offset = *(((int *)old_free)+1); //offset of old_free->distance to free_list
	*(((int *)old_free)+1) = old_offset + change; //offset between old_free and block
	*(((int *)memory_block)+1) = (-1)*change; //offset between block and free_list		
	//printf("end of free.  Memory map: \n");
	//dump_memory_map();
	coalesce_rec();
}



//Carrie and Shreeya
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



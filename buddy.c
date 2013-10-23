/*

Title: Project 03
Purpose: to recreate malloc and free utilizing the buddy system
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

struct node{
	int size;
	//can we have a char to record if node is free or malloced?  Or would that
	//push over the 8 bits?
	int offset;
};
	


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
   
}

void free(void *memory_block) {

}

void dump_memory_map(void) {

}


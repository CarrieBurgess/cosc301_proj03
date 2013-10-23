/*

Title: Project 03
Purpose: to recreate malloc and free utilizing the buddy system
Masterminds: Shreeya Khadka, the Robot from Pluto, and Carrie Burgess, an evil world ruler
Date of world domination (when due): 4 November 2013

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>

void dump_memory_map(void);

int main(int argc, char **argv) {
    void *m1 = malloc(50);  // should allocate 64 bytes
    void *m2 = malloc(100); // should allocate 128 bytes
    free(m1);
    dump_memory_map();
    void *m3 = malloc(56);  // should allocate 64 bytes
    void *m4 = malloc(11);  // should allocate 32 bytes
    free(m3);
    void *m5 = malloc(30);  // should allocate 64 bytes
    void *m6 = malloc(120); // should allocate 128 bytes
    free(m2);
    return 0;
}

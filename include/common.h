#pragma once

#include <stdio.h>
#include <stdlib.h>

#define CLEAR "\e[1;1H\e[2J"

#ifdef DEBON
    /*
        if DEBON is defined you can use the 
        DEBUG macro to display debug info 
        from the console
        fmt = what you want to display 
    */
    #define DEBUG(fmt, ...) printf(fmt, __VA_ARGS__)
#else
    /*
        does nothing if DEBON is not defined
    */
    #define DEBUG(fmt, ...) do { } while(0)
#endif  //DEBON


/*
    print an error message to the console and
    the current line and file then exit.
    fmt = what you want to display to the console
*/
#define ERROR(fmt) do{\
    fprintf(stderr, fmt);\
    printf("\nFILE: %s, LINE %d\n", __FILE__, __LINE__);\
    exit(EXIT_FAILURE);\
} while(0)

void *xcalloc(size_t multiplier, size_t size);
void *xmalloc(size_t size);
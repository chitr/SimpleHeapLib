/*
 * vim:expandtab:shiftwidth=4:tabstop=4:
 *
 * Copyright   (2013)      Contributors
 * Contributor : chitr   chitr.prayatan@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * ---------------------------------------
 */
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/statfs.h>
#include<pthread.h>
#include <inttypes.h>


#define ON  1
#define OFF 0
#define HEAP_SIZE 100000
#define ZERO_KEY 0
#define INFINITE_KEY UINT64_MAX
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef P
#define P(mutex)                                                          \
  do { int rc ;                                                             \
    if( ( rc = pthread_mutex_lock( &mutex ) ) != 0 )                        \
      ;/*TODO:log error */  \
  } while (0)
#endif

#ifndef V
#define V(mutex)                                                          \
  do { int rc ;                                                             \
    if( ( rc = pthread_mutex_unlock( &mutex ) ) != 0 )                      \
      /*TODO:log error */  \
  } while (0)
#endif

#define USE_HEAPLIST_LOCK OFF
#define MAX_TO_APND_RATIO  100

#define LOOKUP_WIDTH_TO_SLEEP 300

typedef enum H_return {
    H_SUCCESS=0,
    H_ERROR,
    H_EMPTY,
    H_FULL,
    H_INPUT_ERROR,
    H_NOTFOUND,
    H_EXISTS,
    H_MEM_ERROR,
    H_RESIZE
}H_return_t;


typedef enum Heap_type{
    H_MAX=0,
    H_MIN
}Heap_type_t;


typedef struct Binary_HeapNode_t__ {
    uint64_t key;
	uint32_t data_tracer;    /*Additional tracer to make lookups faster */
    void *heapdata;
} __attribute__((packed)) Binary_HeapNode_t;

typedef struct Binary_Heap_t__ {
    Binary_HeapNode_t* heap_list;
    uint64_t maxSize;
    uint64_t currSize;
    uint64_t   append_size;
    Heap_type_t type;		
    pthread_mutex_t heap_list_lock;
    int (*compare)(void *, void *);
    int (*release)(void *);
}Binary_Heap_t;




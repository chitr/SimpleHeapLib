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
#include "heap.h"

/*function to get the parent for given index*/
uint64_t parent(uint64_t heap_list_index){
	return (heap_list_index - 1) >> 1;
}
/*function to get the left child for given index*/
uint64_t lchild(uint64_t heap_list_index){
	return (heap_list_index << 1) + 1;
}
/*function to get the right child for given index*/
uint64_t rchild(uint64_t heap_list_index){
	return (heap_list_index << 1) + 2;
}


/*This function builds/resize heap_list of  Binary heap and allocate 
    memory for predefined number of elements */
H_return_t
buildBinHeap(Binary_Heap_t *p_heap,uint64_t heap_size){
    H_return_t ret=H_SUCCESS;
    if(!p_heap ) {
        ret = H_INPUT_ERROR;
        goto end;
    }    
    Binary_HeapNode_t *p_resize_heap_list=NULL;
#if USE_HEAPLIST_LOCK
    P( &(p_heap->heap_list_lock) );
#endif
    if(!(p_resize_heap_list = realloc(p_heap->heap_list,
                    sizeof(Binary_HeapNode_t) * heap_size))) {
        ret = H_MEM_ERROR;
        /*error while resizing the heap*/
#if USE_HEAPLIST_LOCK
        V( &(p_heap->heap_list_lock) );
#endif		
        goto end;		
    }

    p_heap->heap_list=p_resize_heap_list;
    p_heap->maxSize = heap_size;
    if(p_heap->currSize >p_heap->maxSize)
        p_heap->currSize = p_heap->maxSize;
#if USE_HEAPLIST_LOCK
    V( &(p_heap->heap_list_lock) );
#endif
end:
    return ret;
}


/*function deletes the Binary Heap .It is only called when heap size grows
then maximum allowed size*/
H_return_t
deleteBinHeap(Binary_Heap_t* p_heap){   
    H_return_t ret=H_SUCCESS;
    if(!p_heap ) {
        ret = H_INPUT_ERROR;
        goto end;
    }
#if USE_HEAPLIST_LOCK
    P( &(p_heap->heap_list_lock) );
#endif	
    if (p_heap->heap_list != NULL) 
        free(p_heap->heap_list);    
#if USE_HEAPLIST_LOCK
    V( &(p_heap->heap_list_lock) );
#endif	
end:
    return ret;
}


/*Returns the number of elements in Bin heap*/
uint64_t 
elementCount( Binary_Heap_t* p_heap) {
    return p_heap->currSize;
}

/*function used to swap two Nodes in BIN heap /Simple pointer swap*/
H_return_t
swap(Binary_Heap_t* p_heap, uint64_t heap_list_index1 , uint64_t heap_list_index2){
    H_return_t ret=H_SUCCESS;
    if(!p_heap ) {
        ret = H_INPUT_ERROR;
        goto end;
    }
    /*If both index are same ,neednt swap*/
    if(heap_list_index1==heap_list_index2)
        goto end;
    Binary_HeapNode_t temp = p_heap->heap_list[heap_list_index1];
    p_heap->heap_list[heap_list_index1] = p_heap->heap_list[heap_list_index2];
    p_heap->heap_list[heap_list_index2] = temp;

end:	
    return ret;
}

/*function moves up the node to maintain the heap nature*/
H_return_t 
moveUp(Binary_Heap_t* p_heap, uint64_t heap_list_index){
    H_return_t ret=H_SUCCESS;
    if(!p_heap ) {
        ret = H_INPUT_ERROR;
        goto end;
    }
    Heap_type_t heap_type = p_heap->type;
#if USE_HEAPLIST_LOCK
    P( &(p_heap->heap_list_lock) );
#endif	
    /*If it is a root node neednt to move up further*/
    while((heap_list_index > 0) 
            &&((H_MIN==heap_type)?
                /*If MIN heap:cascade-up criteria for MIN heap*/
                (p_heap->heap_list[parent(heap_list_index)].key 
                 > p_heap->heap_list[heap_list_index].key):
                /*Else:cascade-up criteria for MAX heap*/   
                (p_heap->heap_list[parent(heap_list_index)].key 
                 < p_heap->heap_list[heap_list_index].key)			     
              )){
        swap(p_heap, parent(heap_list_index), heap_list_index);
        heap_list_index = parent(heap_list_index);
    }
#if USE_HEAPLIST_LOCK
    V( &(p_heap->heap_list_lock) );
#endif
end:
    return ret;
}

/*function move down the node to maintain the  heap nature*/
H_return_t
moveDown(Binary_Heap_t* p_heap, uint64_t heap_list_index) {
    H_return_t ret=H_SUCCESS;
    if(!p_heap ) {
        ret = H_INPUT_ERROR;
        goto end;
    }
    Heap_type_t heap_type = p_heap->type;
    uint64_t left ,right , lkey ,rkey;
    uint64_t next_heap_list_index =0, next_key; 
    int do_loop = 1;
#if USE_HEAPLIST_LOCK
    P( &(p_heap->heap_list_lock) );
#endif
    /*If current index is last index ,needn't to movedown further*/
    if(heap_list_index == p_heap->currSize -1 )
        goto end;
    while ( do_loop ) {
        /*left child of current index */
        left = lchild(heap_list_index);

        right = rchild(heap_list_index);

        /*If either of left or right */
        /*Incase of left or right has reached to curr size of heap 

          MIN heap : current key value should be min  value and should be compared with Max possible
          value (INFINITE_KEY) and put to next possibe index 
          MAX heap : current key value should be max value and should be compared with MIN Possible 
          and put to next possible index */


        lkey = left < p_heap->currSize ? p_heap->heap_list[left].key : 
            ((H_MIN==heap_type)? INFINITE_KEY : ZERO_KEY) ;
        rkey = right < p_heap->currSize ? p_heap->heap_list[right].key : 
            ((H_MIN==heap_type)? INFINITE_KEY : ZERO_KEY);

        next_heap_list_index = (H_MIN==heap_type)?
            /*cascade-down criteria for MIN heap*/
            ((lkey < rkey) ? left : right):
            /*cascade-down criteria for MAX heap*/
            ((lkey > rkey) ? left : right);

        next_key = (H_MIN==heap_type)?
            /*cascade-down criteria for MIN heap*/
            ((lkey < rkey) ? lkey : rkey):
            /*cascade-down criteria for MAX heap*/
            ((lkey > rkey) ? lkey : rkey);

        if (((H_MIN==heap_type)?
                    (p_heap->heap_list[heap_list_index].key > next_key):
                    (p_heap->heap_list[heap_list_index].key < next_key))) {
            swap(p_heap, heap_list_index, next_heap_list_index);
            heap_list_index = next_heap_list_index;
        } else {
            break;
        }
    }
#if USE_HEAPLIST_LOCK
    V( &(p_heap->heap_list_lock) );
#endif
end:
    return ret;
}
/*function returns the root node */
H_return_t 
getRoot( Binary_Heap_t* p_heap,Binary_HeapNode_t *p_root_node ){
    H_return_t ret=H_SUCCESS;
    if(!p_heap || !p_root_node) {
        ret = H_INPUT_ERROR;
        goto end;
    }
    if (p_heap->currSize == 0){
        /*Heap has no elements return a wild card key value
          and Error H_EMPTY*/

        ret = H_EMPTY;
        goto end;
    }
#if USE_HEAPLIST_LOCK
    P( &(p_heap->heap_list_lock) );
#endif

    p_root_node->key  = p_heap->heap_list[0].key;
    p_root_node->heapdata = p_heap->heap_list[0].heapdata;
#if USE_HEAPLIST_LOCK
    V( &(p_heap->heap_list_lock) );
#endif	
end:
    return ret;
}

/*function deletes the root node from heap.*/
H_return_t 
delRoot(Binary_Heap_t* p_heap ,Binary_HeapNode_t *p_root_node ) {
    H_return_t ret=H_SUCCESS;
    if(!p_heap || !p_root_node) {
        ret = H_INPUT_ERROR;
        goto end;
    }
    if (p_heap->currSize == 0){
        /*Heap has no elements 
          and Error H_EMPTY*/				
        ret = H_EMPTY;
        goto end;
    }

#if USE_HEAPLIST_LOCK
    P( &(p_heap->heap_list_lock) );
#endif

    p_root_node->key  = p_heap->heap_list[0].key;

    p_root_node->heapdata = p_heap->heap_list[0].heapdata;
    /*There should be swap only if there are minimum 2 elements*/
    if(p_heap->currSize > 1 )
        swap(p_heap, 0, p_heap->currSize - 1);
    --(p_heap->currSize);
#if USE_HEAPLIST_LOCK
    P( &(p_heap->heap_list_lock) );
#endif
    ret = moveDown(p_heap, 0);

end:	
    return ret;
}

/*lookup for a value in the heap based on heap data */
/*this is a array lookup as heap node are stored in an array  
    NOTE: This is lookup based on node data not key so it is O(n) order lookup
    data_tracer avoids  the complex comparison functions (based on data)
 */
H_return_t 
lookup_data_in_heap(Binary_Heap_t* p_heap ,Binary_HeapNode_t *p_lookup_node,
        uint64_t *p_index ){
    H_return_t ret=H_SUCCESS;
    if(!p_heap || !p_lookup_node) {
        ret = H_INPUT_ERROR;
        goto end;
    }
    if (p_heap->currSize == 0){
        /*Heap has no elements return a wild card key value
          and Error H_EMPTY*/		
        ret = H_EMPTY;
        goto end;
    }
#if USE_HEAPLIST_LOCK
    P( &(p_heap->heap_list_lock) );
#endif

    register int64_t array_index = p_heap->currSize - 1;
    for(; array_index >= 0; array_index--){
        if(p_lookup_node->data_tracer == p_heap->heap_list[array_index].data_tracer) {
            if (!p_heap->compare(p_lookup_node->heapdata, 
                        p_heap->heap_list[array_index].heapdata)) {
                if(p_index)
                    *p_index = array_index;
#if USE_HEAPLIST_LOCK
                V( &(p_heap->heap_list_lock) );
#endif
                goto end;
            }
        }
        if(array_index && (!(array_index % LOOKUP_WIDTH_TO_SLEEP)))
            usleep(1);

    }
    ret = H_NOTFOUND;
#if USE_HEAPLIST_LOCK
    V( &(p_heap->heap_list_lock) );
#endif	
end:
    return ret;
}

/*delete any node from the heap. */
/*Node is replaced with the last node and and then heap is moved down for heapify*/
H_return_t
delNode(Binary_Heap_t* p_heap ,Binary_HeapNode_t *p_del_node){
    H_return_t ret=H_SUCCESS;
    if(!p_heap || !p_del_node) {
        ret = H_INPUT_ERROR;
        goto end;
    }
    if (p_heap->currSize == 0){
        /*Heap has no elements return a wild card key value
          and Error H_EMPTY*/		
        ret = H_EMPTY;
        goto end;
    }

    uint64_t index = 0;
    if(ret = lookup_data_in_heap( p_heap , p_del_node,&index )){
        goto end;
    }

    p_del_node->heapdata = p_heap->heap_list[index].heapdata;
    p_del_node->key = p_heap->heap_list[index].key;
	p_del_node->data_tracer = p_heap->heap_list[index].data_tracer;
#if USE_HEAPLIST_LOCK
    P( &(p_heap->heap_list_lock) );
#endif
    if((p_heap->currSize-index)>1)
        swap(p_heap, index , p_heap->currSize - 1);

    --(p_heap->currSize);
#if USE_HEAPLIST_LOCK
    V( &(p_heap->heap_list_lock) );
#endif
    ret = moveDown(p_heap, index);
end:
    return ret;

}

/*Function inserts in the heap */
H_return_t
insertNode(Binary_Heap_t* p_heap ,Binary_HeapNode_t *p_in_node,int *p_is_lookup_req, int *p_overwrite) {
	H_return_t ret=H_SUCCESS;
    if(!p_in_node) {
        ret = H_INPUT_ERROR;
        goto end;
    }
    uint64_t  len_to_resize = p_heap->maxSize+p_heap->append_size;
    uint64_t index = -1;
    int do_resize=FALSE;

    /*expanding the heap*/
    if((p_heap->maxSize == 0) || (p_heap->currSize >= p_heap->maxSize)) {
        do_resize =TRUE;
        len_to_resize = p_heap->maxSize+p_heap->append_size;
    }else if((p_heap->maxSize / 
                ((p_heap->append_size > p_heap->currSize) ? 
                 p_heap->append_size : p_heap->currSize)) > MAX_TO_APND_RATIO) {
        /*We  should resize heap if maxSize >>>append_size ,say 100 times
          i.e max_size = 100 00 000 and append_size = 100000 let free some memory by resizing
          heap to half
          size_to_resize = p_heap->max_size / 2*/
        len_to_resize = p_heap->maxSize / 2;
        do_resize =TRUE;
    }
    if ((do_resize )){				
        if(ret = buildBinHeap(p_heap,len_to_resize))
            goto end;
    }

    if(TRUE == *p_is_lookup_req){
        if(H_SUCCESS == (lookup_data_in_heap(p_heap ,p_in_node,&index)) ) {
            if (!p_overwrite || !(*p_overwrite)) {
                ret= H_EXISTS;
                goto end;
            }
        }
    }

    if (index == -1) {
        if (p_overwrite)
            *p_overwrite = 0;
        /*Add to new to end of heap-list and cascade up*/	
#if USE_HEAPLIST_LOCK
        P( &(p_heap->heap_list_lock) );
#endif
        p_heap->heap_list[p_heap->currSize].heapdata = p_in_node->heapdata;
        p_heap->heap_list[p_heap->currSize].key = p_in_node->key;
        p_heap->heap_list[p_heap->currSize].data_tracer = p_in_node->data_tracer;

#if USE_HEAPLIST_LOCK
        V( &(p_heap->heap_list_lock) );
#endif

        uint64_t heap_list_index = (p_heap->currSize)++;
        ret = moveUp(p_heap, heap_list_index);	
    } else {
#if USE_HEAPLIST_LOCK
        P( &(p_heap->heap_list_lock) );
#endif
        int increased = (p_heap->heap_list[index].key < p_in_node->key);

        if (p_heap->release) {
            p_heap->release(&p_heap->heap_list[index]);
        }
        p_heap->heap_list[index].heapdata = p_in_node->heapdata;
        p_heap->heap_list[index].key = p_in_node->key;
        p_heap->heap_list[index].data_tracer = p_in_node->data_tracer;

#if USE_HEAPLIST_LOCK
        V( &(p_heap->heap_list_lock) );
#endif

        if ((p_heap->type == H_MIN && increased) || 
                (p_heap->type == H_MAX && !increased)) {
            ret = moveDown(p_heap, index);	
        } else {
            ret = moveUp(p_heap, index);
        }
    }
end:
    return ret;
}


/*
*   DKU Operating System Lab
*           Lab2 (Hash Queue Lock Problem)
*           Student id : 32201669
*           Student name : Park Se Yeon (박세연)
*
*   lab2_sync.c :
*       - lab2 main file.
*		- Thread-safe Hash Queue Lock code
*		- Coarse-grained, fine-grained lock code
*       - Must contains Hash Queue Problem function's declations.
*
*   Implement thread-safe Hash Queue for coarse-grained verison and fine-grained version.
*/

#include <aio.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>

#include "lab2_sync_types.h"

/*
 * TODO
 *  Implement function which init queue nodes for front and rear
 *  ( refer to the ./include/lab2_sync_types.h for front and rear nodes)
 */
void init_queue() {
    front = malloc(sizeof(queue_node));
    rear = malloc(sizeof(queue_node));
    
    front->next = NULL;
    front->prev = NULL;
    front->data = 0;
    rear->next = NULL;
    rear->next = NULL;
    rear->data = 0;
    
}

/*
 * TODO
 *  Implement function which add new_node at next rear node
 *
 *  @param queue_node *new_node		: Node which you need to insert at queue.
 */
void enqueue(queue_node *new_node) {
    assert(new_node != NULL);
    if (front == NULL) {
        front = new_node;
        rear = new_node;
    }else{
        rear->next = new_node;
        new_node->prev = rear;
    }
    rear = new_node;
}
/*
 * TODO
 *  Implement function which add new_node at next rear node
 *
 *  @param queue_node *new_node		: Node which you need to insert at queue in coarse-grained manner.
 */
void enqueue_cg(queue_node *new_node) {
    pthread_mutex_lock(&L1);
    assert(new_node != NULL);
    
    if (front == NULL) {
        front = new_node;
        rear = new_node;
    }else{
        rear->next = new_node;
        new_node->prev = rear;
    }
    rear = new_node;
    pthread_mutex_unlock(&L1);
}
/*
 * TODO
 *  Implement function which add new_node at next rear node
 *
 *  @param queue_node *new_node		: Node which you need to insert at queue in fine-grained manner.
 */
void enqueue_fg(queue_node *new_node) {
    assert(new_node != NULL);
    if (front == NULL) {
        pthread_mutex_lock(&L1);
        front = new_node;
        pthread_mutex_unlock(&L1);
        
        pthread_mutex_lock(&L1);
        rear = new_node;
        pthread_mutex_unlock(&L1);
        
    }else{
        pthread_mutex_lock(&L1);
        rear->next = new_node;
        pthread_mutex_unlock(&L1);
        
        pthread_mutex_lock(&L1);
        new_node->prev = rear;
        pthread_mutex_unlock(&L1);
        
    }
    pthread_mutex_lock(&L1);
    rear = new_node;
    pthread_mutex_unlock(&L1);
}

/*
 * TODO
 *  Implement function which delete del_node at location that contains target key
 *
 *  @param queue_node *del_node		: Node which you need to delete at queue.
 */
void dequeue(queue_node *del_node) {
    if (front == rear) {
    }else if(del_node->prev == NULL){
        front = del_node->next;
        del_node->next->prev = NULL;
    }else if (del_node->next == NULL){
        rear = del_node->prev;
        del_node->prev->next = NULL;
    }else{
        del_node->next->prev = del_node->prev;
        del_node->prev->next = del_node->next;
    }
    free(del_node);

}
/*
 * TODO
 *  Implement function which delete del_node at location that contains target key
 *
 *  @param queue_node *del_node		: Node which you need to delete at queue in coarse-grained manner.
 */
void dequeue_cg(queue_node *del_node) {
    pthread_mutex_lock(&L2);
    if (front == rear) {
    }else if(del_node->prev == NULL){
        front = del_node->next;
        del_node->next->prev = NULL;
    }else if (del_node->next == NULL){
        rear = del_node->prev;
        del_node->prev->next = NULL;
    }else{
        del_node->next->prev = del_node->prev;
        del_node->prev->next = del_node->next;
    }
    free(del_node);
    pthread_mutex_unlock(&L2);
}

/*
 * TODO
 *  Implement function which delete del_node at location that contains target key
 *
 *  @param queue_node *del_node		: Node which you need to delete at queue in fine-grained manner.
 */
void dequeue_fg(queue_node *del_node) {
    if (front == rear) {
    }else if(del_node->prev == NULL){
        pthread_mutex_lock(&L2);
        front = del_node->next;
        pthread_mutex_unlock(&L2);
        
        pthread_mutex_lock(&L2);
        del_node->next->prev = NULL;
        pthread_mutex_unlock(&L2);
        
    }else if (del_node->next == NULL){
        pthread_mutex_lock(&L2);
        rear = del_node->prev;
        pthread_mutex_unlock(&L2);
        
        pthread_mutex_lock(&L2);
        del_node->prev->next = NULL;
        pthread_mutex_unlock(&L2);
        
    }else{
        pthread_mutex_lock(&L2);
        del_node->next->prev = del_node->prev;
        pthread_mutex_unlock(&L2);
        
        pthread_mutex_lock(&L2);
        del_node->prev->next = del_node->next;
        pthread_mutex_unlock(&L2);
    }
    free(del_node);
}

/*
 * TODO
 *  Implement function which init hashlist(same as hashtable) node.
 */
void init_hlist_node() {
    int i = 0;
    for (; i<HASH_SIZE; i++) {
        hashlist[i] = malloc(sizeof(hlist_node));
        hashlist[i] = NULL;
    }
}

/*
 * TODO
 *  Implement function which calculate hash value with modulo operation.
 */
int hash(int val) {
    int hv = val % HASH_SIZE;
    if (hv < 0) {
        hv += HASH_SIZE;
    }
    return hv;
}

/*
 * TODO
 *  Implement function which insert the resilt of finding the location 
 *  of the bucket using value to the entry and hashtable
 *
 *  @param hlist_node *hashtable		: A pointer variable containing the bucket
 *  @param int val						: Data to be stored in the queue node
 */
void hash_queue_add(hlist_node **hashtable, int val) {
    hlist_node * new_hlist_node = malloc(sizeof(hlist_node));
    queue_node * new_node = malloc(sizeof(queue_node));
    
    new_node->data = val;
    enqueue(new_node);
    new_hlist_node->q_loc = new_node;
    
    new_hlist_node->next = *hashtable;
    *hashtable = new_hlist_node;
}
/*
 * TODO
 *  Implement function which insert the resilt of finding the location 
 *  of the bucket using value to the entry and hashtable
 *
 *  @param hlist_node *hashtable		: A pointer variable containing the bucket
 *  @param int val						: Data to be stored in the queue node
 */
void hash_queue_add_cg(hlist_node **hashtable, int val) {
    hlist_node * new_hlist_node = malloc(sizeof(hlist_node));
    queue_node * new_node = malloc(sizeof(queue_node));
    
    pthread_mutex_lock(&L3);
    new_node->data = val;
    enqueue_cg(new_node);
    new_hlist_node->q_loc = new_node;
    
    new_hlist_node->next = *hashtable;
    *hashtable = new_hlist_node;
    pthread_mutex_unlock(&L3);
}

/*
 * TODO
 *  Implement function which insert the resilt of finding the location 
 *  of the bucket using value to the entry and hashtable
 *
 *  @param hlist_node *hashtable		: A pointer variable containing the bucket
 *  @param int val						: Data to be stored in the queue node
 */
void hash_queue_add_fg(hlist_node **hashtable, int val) {
    hlist_node * new_hlist_node = malloc(sizeof(hlist_node));
    queue_node * new_node = malloc(sizeof(queue_node));
    
    pthread_mutex_lock(&L3);
    new_node->data = val;
    pthread_mutex_unlock(&L3);
    
    enqueue_fg(new_node);
    
    pthread_mutex_lock(&L3);
    new_hlist_node->q_loc = new_node;
    pthread_mutex_unlock(&L3);
    
    pthread_mutex_lock(&L3);
    new_hlist_node->next = *hashtable;
    pthread_mutex_unlock(&L3);
    
    pthread_mutex_lock(&L3);
    *hashtable = new_hlist_node;
    pthread_mutex_unlock(&L3);
}

/*
 * TODO
 *  Implement function which check if the data(value) to be stored is in the hashtable
 *
 *  @param int val						: variable needed to check if data exists
 *  @return								: status (success or fail)
 */
int value_exist(int val) {
    int h = hash(val);
    hlist_node * tmp = malloc(sizeof(hlist_node));
    
    tmp = hashlist[h];
    while (tmp != NULL) {
        if (tmp->q_loc->data == val) {
            return 1;
        }
        tmp = tmp->next;
    }
//    free(tmp);
    return 0;
}

/*
 * TODO
 *  Implement function which find the bucket location using target
 */
void hash_queue_insert_by_target() {
    int t = hash(target);
    if (value_exist(target) == 0) {
        hash_queue_add(&hashlist[t], target);
    }
}

/*
 * TODO
 *  Implement function which find the bucket location using target
 */
void hash_queue_insert_by_target_cg() {
    pthread_mutex_lock(&L4);
    int t = hash(target);
    if (value_exist(target) == 0) {
        hash_queue_add_cg(&hashlist[t], target);
    }
    pthread_mutex_unlock(&L4);
}

/*
 * TODO
 *  Implement function which find the bucket location using target
 */
void hash_queue_insert_by_target_fg() {
    pthread_mutex_lock(&L4);
    int t = hash(target);
    pthread_mutex_unlock(&L4);
    
    if (value_exist(target) == 0) {
        pthread_mutex_lock(&L4);
        hash_queue_add_fg(&hashlist[t], target);
        pthread_mutex_unlock(&L4);
    }
}

/*
 * TODO
 *  Implement function which find the bucket location and stored data
 *  using target and delete node that contains target
 */
void hash_queue_delete_by_target() {
    int h = hash(target);
    hlist_node *tmp = malloc(sizeof(hlist_node));
    hlist_node *tmp2 = malloc(sizeof(hlist_node));
    
    tmp = hashlist[h];
    
    if (tmp == NULL) {
        return;
    }
    if (tmp->q_loc->data == target) {
        tmp2 = hashlist[h]->next;
        free(hashlist[h]);
        hashlist[h] =tmp2;
        return;
    }
    while (tmp->next != NULL) {
        if (tmp->next->q_loc->data == target) {
            tmp2 = tmp->next->next;
            dequeue(tmp->next->q_loc);
            free(tmp->next);
            tmp->next = tmp2;
            return;
        }
        tmp = tmp->next;
    }
//    free(tmp);
//    free(tmp2);
}

/*
 * TODO
 *  Implement function which find the bucket location and stored data
 *  using target and delete node that contains target
 */
void hash_queue_delete_by_target_cg() {
    int h = hash(target);
    hlist_node *tmp = malloc(sizeof(hlist_node));
    hlist_node *tmp2 = malloc(sizeof(hlist_node));
    
    tmp = hashlist[h];
    
    if (tmp == NULL) {
        return;
    }
    if (tmp->q_loc->data == target) {
        tmp2 = hashlist[h]->next;
        free(hashlist[h]);
        hashlist[h] =tmp2;
        return;
    }
    while (tmp->next != NULL) {
        if (tmp->next->q_loc->data == target) {
            tmp2 = tmp->next->next;
            dequeue_cg(tmp->next->q_loc);
            free(tmp->next);
            tmp->next = tmp2;
            return;
        }
        tmp = tmp->next;
    }
//    free(tmp);
//    free(tmp2);
}

/*
 * TODO
 *  Implement function which find the bucket location and stored data
 *  using target and delete node that contains target
 */
void hash_queue_delete_by_target_fg() {
    int h = hash(target);
    hlist_node *tmp = malloc(sizeof(hlist_node));
    hlist_node *tmp2 = malloc(sizeof(hlist_node));
    
    tmp = hashlist[h];
    
    if (tmp == NULL) {
        return;
    }
    if (tmp->q_loc->data == target) {
        tmp2 = hashlist[h]->next;
        free(hashlist[h]);
        hashlist[h] =tmp2;
        return;
    }
    while (tmp->next != NULL) {
        if (tmp->next->q_loc->data == target) {
            tmp2 = tmp->next->next;
            dequeue_fg(tmp->next->q_loc);
            free(tmp->next);
            tmp->next = tmp2;
            return;
        }
        tmp = tmp->next;
    }
//    free(tmp);
//    free(tmp2);
}


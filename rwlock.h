//
// Created by Tim McVicker on 10/10/16.
//

#ifndef PROJECT2_RWLOCK_H
#define PROJECT2_RWLOCK_H
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>

typedef struct node_def{
    int is_writer;
    sem_t waiting;
    struct node_def *next;
} node_t;

typedef struct{
    sem_t mutex;
    int is_writer;
    int read_count;
    node_t *head;
    node_t *tail;
} fcfsrwlock_t;

fcfsrwlock_t *create_lock();
void destroy_lock( fcfsrwlock_t *lock );
void get_read_access( fcfsrwlock_t *lock );
void release_read_access( fcfsrwlock_t *lock );
void get_write_access( fcfsrwlock_t *lock );
void release_write_access( fcfsrwlock_t *lock );

#endif //PROJECT2_RWLOCK_H

//
// Created by Tim McVicker on 10/10/16.
//

#include"rwlock.h"

node_t *create_node() {
    int rc;
    node_t *node = malloc(sizeof(node_t));
    if (node == NULL) {
        printf("** create node failed\n");
        exit(-1);
    }
    rc = sem_init(&(node->waiting), 0, 0);
    if (rc) {
        printf("** init node->sem failed\n");
        exit(-1);
    }
    node->is_writer = 0;
    node->next = NULL;
    return node;
}

void destroy_node(node_t *node) {
    sem_destroy(&(node->waiting));
    free(node);
}

fcfsrwlock_t *create_lock() {
    int rc;
    fcfsrwlock_t *lock = malloc(sizeof(fcfsrwlock_t));
    if (lock == NULL) {
        printf("** create lock failed\n");
        exit(-1);
    }
    rc = sem_init(&(lock->mutex), 0, 1);
    if (rc) {
        printf("** init lock->sem failed\n");
        exit(-1);
    }
    lock->is_writer = 0;
    lock->read_count = 0;
    lock->head = lock->tail = NULL;
    return lock;
}

void destroy_lock(fcfsrwlock_t *lock) {
    node_t *ptr;
    sem_destroy(&(lock->mutex));
    while (lock->head != NULL) {
        ptr = lock->head;
        lock->head = lock->head->next;
        destroy_node(ptr);
    }
    free(lock);
}

// helper function called within a CS
// - assumes caller waits and posts mutex of lock
// - creates a node with is-writer field set to 0
//   and places node on tail of queue
node_t *enqueue_reader(fcfsrwlock_t *lock) {
    sem_post(&(lock->mutex));
    node_t *nodeToEnqueue = create_node();
    if (lock->head == NULL) {
        lock->head = nodeToEnqueue;
        lock->tail = nodeToEnqueue;
    } else {
        lock->tail->next = nodeToEnqueue;
        lock->tail = nodeToEnqueue;
    }

    return nodeToEnqueue;
}

// helper function called within a CS
// - assumes caller waits and posts mutex of lock
// - creates a node with is-writer field set to 1
//   and places node on tail of queue
node_t *enqueue_writer(fcfsrwlock_t *lock) {
    sem_post(&(lock->mutex));
    node_t *nodeToEnqueue = create_node();
    nodeToEnqueue->is_writer = 1;
    if (lock->head == NULL) {
        lock->head = nodeToEnqueue;
        lock->tail = nodeToEnqueue;
    } else {
        lock->tail->next = nodeToEnqueue;
        lock->tail = nodeToEnqueue;
    }
    return nodeToEnqueue;
}

// helper function called within a CS
// - assumes caller waits and posts mutex of lock
// - immediately returns if queue is empty, otherwise removes node
//   from head of list, sets is_writer field of lock to is_writer
//   value of node, increments read_count of lock if a reader node,
//   and posts the semaphore in the node in order to resume the
//   blocked thread
// - if the node was a reader and while there are additional reader
//   nodes at the head of the queue, repeats the process of removing
//    a reader node, incrementing read_count, and posting
void wakeup_waiting_threads(fcfsrwlock_t *lock) {
    sem_post(&(lock->mutex));
    if (lock->head == NULL) {
        return;
    }

    node_t *nodeToDequeue = lock->head;
    do {
        lock->head = nodeToDequeue->next;
        lock->is_writer = nodeToDequeue->is_writer;
        if (lock->is_writer == 0) {
            lock->read_count++;
        }

        sem_post(&(nodeToDequeue->waiting));
        nodeToDequeue = lock->head;
    } while (nodeToDequeue->is_writer == 0);
}

// protocol for starting a read
// - waits on mutex of lock
// - if there is no current writer and nothing in queue, then
//   increments read_count of lock, posts mutex, and returns
// - otherwise, enqueues a node on the queue, posts mutex of
//   lock, and waits on sempahore in node; when resumed
//   destroys the node and returns
void get_read_access(fcfsrwlock_t *lock) {
    sem_wait(&(lock->mutex));

    if (lock->is_writer == 0 && lock->head == NULL) {
        lock->read_count++;
        sem_post(&(lock->mutex));

        return;
    } else {
        node_t* nodeFromEnqueue = enqueue_reader(lock);
        sem_post(&(lock->mutex));
        sem_wait(&(nodeFromEnqueue->waiting));
        destroy_node(nodeFromEnqueue);

        return;
    }
}

// protocol for ending a read
// - waits on mutex of lock
// - decrements read_count and if read_count is zero then calls
//   wakeup_waiting_threads() to resume blocked threads
// - posts mutex
void release_read_access(fcfsrwlock_t *lock) {
    sem_wait(&(lock->mutex));
    lock->read_count--;
    if (lock->read_count == 0) {
        wakeup_waiting_threads(lock);
    }

    sem_post(&(lock->mutex));
}

// protocol for starting a write
// - waits on mutex of lock
// - if there is no current writer or current reader(s), then
//   sets is_writer field of lock, posts mutex, and returns
// - otherwise, enqueues a node on the queue, posts mutex of
//   lock, and waits on sempahore in node; when resumed
//   destroys the node and returns
void get_write_access(fcfsrwlock_t *lock) {
    sem_wait(&(lock->mutex));
    if (lock->is_writer == 0 && lock->read_count == 0) {
        lock->is_writer = 1;
        sem_post(&(lock->mutex));

        return;
    } else {
        node_t* nodeFromEnqueue = enqueue_writer(lock);
        sem_post(&(lock->mutex));
        sem_wait(&(nodeFromEnqueue->waiting));
        destroy_node(nodeFromEnqueue);

        return;
    }
}

// protocol for ending a write
// - waits on mutex of lock
// - resets is_writer field of lock and calls
//   wakeup_waiting_threads() to resume blocked threads
// - posts mutex
void release_write_access(fcfsrwlock_t *lock) {
    sem_wait(&(lock->mutex));
    lock->is_writer = 0;
    wakeup_waiting_threads(lock);
    sem_post(&(lock->mutex));
}
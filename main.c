//
// Created by Tim McVicker on 10/10/16.
//

#include"rwlock.h"

#define MAX_THREADS 20

fcfsrwlock_t *lock;

void *reader( void *thread_id ){
    get_read_access( lock );
    printf( "thread %ld starts read\n", (long)thread_id );
    sleep( 4 );
    printf( "thread %ld ends read\n", (long)thread_id );
    release_read_access( lock );
    return NULL;
}

void *writer( void *thread_id ){
    get_write_access( lock );
    printf( "thread %ld starts write\n", (long)thread_id );
    sleep( 4 );
    printf( "thread %ld ends write\n", (long)thread_id );
    release_write_access( lock );
    return NULL;
}

int main( int argc, char **argv ){
    pthread_t threads[MAX_THREADS];
    int i;
    int rc;
    int thread_count = 0;

    if( argc != 2 ){
        printf( "reader-writer string should be provided\n" );
        printf( "as a command argument, e.g., RRWR\n" );
        exit( 0 );
    }

    lock = create_lock();

    for( i = 0; i < strlen( argv[1] ); i++ ){

        if( thread_count == MAX_THREADS ) break;

        if( argv[1][i] == 'R'){
            rc = pthread_create( &threads[thread_count], NULL, &reader,
                                 (void *)((long)thread_count) );
        }else if( argv[1][i] == 'W'){
            rc = pthread_create( &threads[thread_count], NULL, &writer,
                                 (void *)((long)thread_count) );
        }
        if( rc ){ printf( "** could not create thread %d\n", i ); exit( -1 ); }
        thread_count++;
        sleep( 1 );
    }

    for( i = 0; i < thread_count; i++ ){
        rc = pthread_join(threads[i], NULL);
        if( rc ){ printf("** could not join thread %d\n", i); exit( -1 ); }
    }

    destroy_lock( lock );

    return 0;
}
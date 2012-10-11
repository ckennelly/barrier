/**
 * Copyright (C) 2012 Chris Kennelly <chris@ckennelly.com>
 *
 * Use, modification, and distribution are subject to the Boost Software
 * License, Version 1.0. (See accompanying file LICENSE.BOOST or a copy at
 * <http://www.boost.org/LICENSE_1_0.txt>.)
 */

#define __USE_XOPEN2K /* For pthread_barrier_* */
#include <pthread.h>
#include "barrier.h"
#include <assert.h>
#include <libgen.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "timer.h"

typedef struct {
    int passes;
    uint64_t total[3];
    lock_barrier_t * lock;
    pthread_barrier_t * pthread;
    minibarrier_t * mini;
} worker_argument_t;

void * worker(void * arg_) {
    worker_argument_t * const arg = arg_;

    arg->total[0] = 0;
    arg->total[1] = 0;
    arg->total[2] = 0;
    const int strides = arg->passes >> 3;

    assert(arg->lock);
    assert(arg->pthread);
    assert(arg->mini);

    /* Wait for the other workers. */
    lock_barrier_wait(arg->lock);

    for (int i = 0; i < strides; i++) {
        const uint64_t start = timer();

        lock_barrier_wait(arg->lock);
        lock_barrier_wait(arg->lock);
        lock_barrier_wait(arg->lock);
        lock_barrier_wait(arg->lock);
        lock_barrier_wait(arg->lock);
        lock_barrier_wait(arg->lock);
        lock_barrier_wait(arg->lock);
        lock_barrier_wait(arg->lock);

        const uint64_t end   = timer();

        arg->total[0] += end - start;
    }

    pthread_barrier_wait(arg->pthread);

    for (int i = 0; i < strides; i++) {
        const uint64_t start = timer();

        pthread_barrier_wait(arg->pthread);
        pthread_barrier_wait(arg->pthread);
        pthread_barrier_wait(arg->pthread);
        pthread_barrier_wait(arg->pthread);
        pthread_barrier_wait(arg->pthread);
        pthread_barrier_wait(arg->pthread);
        pthread_barrier_wait(arg->pthread);
        pthread_barrier_wait(arg->pthread);

        const uint64_t end   = timer();

        arg->total[1] += end - start;
    }

    minibarrier_wait(arg->mini);

    for (int i = 0; i < strides; i++) {
        const uint64_t start = timer();

        minibarrier_wait(arg->mini);
        minibarrier_wait(arg->mini);
        minibarrier_wait(arg->mini);
        minibarrier_wait(arg->mini);
        minibarrier_wait(arg->mini);
        minibarrier_wait(arg->mini);
        minibarrier_wait(arg->mini);
        minibarrier_wait(arg->mini);

        const uint64_t end   = timer();

        arg->total[2] += end - start;
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s threads\n", basename(argv[0]));
        return 1;
    }

    int n_threads;
    if (sscanf(argv[1], "%d", &n_threads) != 1) {
        fprintf(stderr, "Could not parse '%s' as an integer.\n", argv[1]);
        return 2;
    }

    if (n_threads < 1) {
        fprintf(stderr, "The number of threads must be positive.\n");
        return 3;
    }

    pthread_t * threads = malloc((unsigned) n_threads * sizeof(*threads));
    if (!(threads)) {
        fprintf(stderr, "Unable to allocate thread list.\n");
        return 4;
    }

    worker_argument_t * args = malloc((unsigned) n_threads * sizeof(*args));
    if (!(args)) {
        free(threads);

        fprintf(stderr, "Unable to allocate thread arguments.\n");
        return 5;
    }

    lock_barrier_t lbarrier;
    lock_barrier_init(&lbarrier, n_threads);

    pthread_barrier_t pbarrier;
    /* TODO Check error */
    pthread_barrier_init(&pbarrier, NULL, (unsigned) n_threads);

    minibarrier_t mbarrier;
    minibarrier_init(&mbarrier, n_threads);

    const int passes = 1 << 18;

    for (int i = 0; i < n_threads; i++) {
        args[i].passes  = passes;
        args[i].lock    = &lbarrier;
        args[i].pthread = &pbarrier;
        args[i].mini    = &mbarrier;

        assert(args[i].lock);
        assert(args[i].pthread);
        assert(args[i].mini);

        int ret = pthread_create(&threads[i], NULL, worker, &args[i]);
        if (ret) {
            /* This may leak. */
            fprintf(stderr, "Unable to create thread %d.\n", i);
            return 6;
        }
    }

    for (int i = 0; i < n_threads; i++) {
        /* Skip checking the return code as we can't do much to recover
         * from problems here. */
        pthread_join(threads[i], NULL);
    }

    free(threads);

    const char * method0 = "lock and spin-based barrier";
    const char * method1 = "pthreads-based             ";
    const char * method2 = "minibarriers               ";
    const char * methods[3] = {method0, method1, method2};

    uint64_t sum[3] = {0, 0, 0};
    printf("All units of time are in arbitrary units.  "
        "Performing %d passes.\n", passes);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < n_threads; j++) {
            sum[i] += args[j].total[i];
        }
        sum[i] /= (unsigned) n_threads;

        /**
         * GCC -Wformat is unhappy with PRIu64, so we attempt to approximate it
         * for x86_64.
         */
        printf("Total time with %s %lu (average)\n", methods[i], sum[i]);
    }

    free(args);

    return 0;
}

/**
 * Copyright (C) 2012 Chris Kennelly <chris@ckennelly.com>
 *
 * Use, modification, and distribution are subject to the Boost Software
 * License, Version 1.0. (See accompanying file LICENSE.BOOST or a copy at
 * <http://www.boost.org/LICENSE_1_0.txt>.)
 */

/**
 * These barriers are available for inlining by the compiler.
 */
#ifndef __BARRIER__BARRIER_H__
#define __BARRIER__BARRIER_H__

#include <pthread.h>

#ifdef __CPLUSPLUS
extern "C" {
#endif

typedef struct {
    int spin;
    int remaining;
    int threads;
    volatile int flag;
} lock_barrier_t;

void lock_barrier_init(lock_barrier_t * barrier, int threads);

static inline int atomic_increment(int * p) {
#if (defined(__GNUC__) || defined(__ICC))
    return __sync_add_and_fetch(p, 1);
#else
#error No atomic implementation provided for this compiler.
#endif
}

static inline int atomic_cas(int * p, int oldval, int newval) {
#if (defined(__GNUC__) || defined(__ICC))
    return __sync_val_compare_and_swap(p, oldval, newval);
#else
#error No CAS implementation provided for this compiler.
#endif
}

static inline void lock(int * l) {
    while (atomic_cas(l, 0, 1)) {
        __asm__ __volatile__("pause");
    }
}

static inline void unlock(int * l) {
    *l = 0;
    __asm__ __volatile__("sfence");
}

static inline int lock_barrier_wait(lock_barrier_t * barrier) {
    lock(&barrier->spin);

    int result = 0;

    if (!(--barrier->remaining)) {
        /* Last thread. */
        result = 1;
        barrier->flag++;
    } else {
        int old_flag = barrier->flag;

        unlock(&barrier->spin);
        do {
            __asm__ __volatile__("pause");
        } while (old_flag == barrier->flag);
    }

    if (atomic_increment(&barrier->remaining) == barrier->threads) {
        /* All threads are out except this one, unlock. */
        unlock(&barrier->spin);
    }

    return result;
}

typedef struct {
    int flag;
    int threads;
    int barrier[2];
} minibarrier_t;

void minibarrier_init(minibarrier_t * barrier, int threads);

/**
 * This barrier implementation does not support the case where more threads
 * are interacting with the barrier than the width of the barrier itself.
 */
static inline int minibarrier_wait(minibarrier_t * barrier) {
    int flag = barrier->flag;

    if (atomic_increment(&barrier->barrier[flag]) == barrier->threads) {
        /**
         * Last thread.  Reset barrier and turn.
         * 
         * The order of this operation should be unimportant, since under the
         * assumption that only barrier->threads are interacting with the
         * barrier, the second barrier will not be passed until this thread
         * exits and completes reseting the first barrier.
         */
        barrier->flag          = flag ^ 0x1;
        barrier->barrier[flag] = 0;

        return 1;
    } else {
        /* Spin until the barrier turns. */
        do {
            __asm__ __volatile__("pause");
        } while (flag == *(volatile int *) &barrier->flag);

        return 0;
    }
}

#ifdef __CPLUSPLUS
}
#endif

#endif // __BARRIER__BARRIER_H__

/**
 * Copyright (C) 2012 Chris Kennelly <chris@ckennelly.com>
 *
 * Use, modification, and distribution are subject to the Boost Software
 * License, Version 1.0. (See accompanying file LICENSE.BOOST or a copy at
 * <http://www.boost.org/LICENSE_1_0.txt>.)
 */

#include "barrier.h"

void lock_barrier_init(lock_barrier_t * b, int threads) {
    b->spin         = 0;
    b->remaining    = threads;
    b->threads      = threads;
    b->flag         = 0;
}

void minibarrier_init(minibarrier_t * b, int threads) {
    b->flag         = 0;
    b->threads      = threads;
    b->barrier[0]   = 0;
    b->barrier[1]   = 0;
}


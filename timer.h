/**
 * Copyright (C) 2012 Chris Kennelly <chris@ckennelly.com>
 *
 * Use, modification, and distribution are subject to the Boost Software
 * License, Version 1.0. (See accompanying file LICENSE.BOOST or a copy at
 * <http://www.boost.org/LICENSE_1_0.txt>.)
 */

/**
 * This exposes access to whatever high precision fast timing mechancism is
 * available on the underlying system.
 */
#ifndef __BARRIER__TIMER_H__
#define __BARRIER__TIMER_H__

#include <stdint.h>

#ifdef __BARRIER__TIMER
#undef __BARRIER__TIMER
#endif

#ifdef __CPLUSPLUS
extern "C" {
#endif

#if (defined(__GNUC__) || defined(__ICC))

#if   defined(__i386__)

static inline uint64_t timer(void) {
    uint64_t ret;
    __asm__ __volatile__("rdtsc": "=A" (ret));
    return ret;
}

#define __BARRIER__TIMER
#elif defined(__x86_64__)

static inline uint64_t timer(void) {
    uint32_t ret_upper, ret_lower;
    __asm__ __volatile__("rdtsc": "=a" (ret_lower), "=d" (ret_upper));
    return ((uint64_t) ret_lower) | (((uint64_t) ret_upper) << 32);
}

#define __BARRIER__TIMER
#endif

#endif


#ifdef __BARRIER__TIMER
#undef __BARRIER__TIMER
#else
#error No adequate timing mechancism is available.
#endif

#ifdef __CPLUSPLUS
}
#endif

#endif // __BARRIER__TIMER_H__

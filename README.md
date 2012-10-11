Low-level Barrier Implementations
(c) 2012 - Chris Kennelly (chris@ckennelly.com)

Overview
========

In parallel software, synchronization barriers between threads may be
necessary.  Depending on their frequency, they may carry a non-trivial and
undesirable performance impact.  This source tree implements three approaches
and a test runner to time them.

For simplicity, this code is licensed under the Boost Software License.

Implementations
===============

pthreads
--------

As a baseline, the performance of `pthread_barrier_wait` is assessed against
the implementations here.

Lock and spin-based barrier
---------------------------

This barrier uses atomic swaps on `spin` to enforce mutual exclusion on the
counter `remaining`.  The barrier action proceeds in two phases.
* For the first threads arriving to the barrier, they proceed to wait by
  spinning until `flag` advances.  For the last thread, `flag` is advanced
  releasing the threads waiting at the barrier.
* Once released, the threads increment `remaining` atomically and the last
  thread out unlocks the barrier for further use.

Minibarriers
--------------

This barrier alternates between two counters.  After a counter reaches the
width of the barrier, `flag` is flipped, releasing the waiting threads, and the
counter is reset.

The use of two partial barriers allows released threads to exit the barrier
without further synchronization (for instance, the exit turnstile of the lock
and spin barrier above) and proceed to re-enter.

The lack of synchronization makes this barrier *unsafe* when more threads are
interacting with the barrier than its width.

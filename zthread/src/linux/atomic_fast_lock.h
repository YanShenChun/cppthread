/*
 * Copyright (c) 2005, Eric Crahen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __ZTFASTLOCK_H__
#define __ZTFASTLOCK_H__

#include <assert.h>
#include "../thread_ops.h"
#include "zthread/non_copyable.h"

#if !defined(NDEBUG)
#include <pthread.h>
#endif

namespace zthread {

/**
 * @class FastLock
 *
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T23:27:03-0400>
 * @version 2.2.0
 *
 * This implementation of a FastLock uses the atomic operations that
 * linux provides with its kernel sources. This demonstrates how to implement
 * a spinlock with a decrement and test primative.
 */
class FastLock : private NonCopyable {
  int _value;

#if !defined(NDEBUG)
  pthread_t _owner;
#endif

 public:
  inline FastLock() {
    _value = 1;
  }

  inline ~FastLock() {
    assert(_value == 1);
    assert(_owner == 0);
  }

  inline void Acquire() {
    while (__sync_sub_and_fetch(reinterpret_cast<int*>(&_value), 1) != 0) {
      __sync_add_and_fetch(reinterpret_cast<int*>(&_value), 1);
      ThreadOps::yield();
    }

#if !defined(NDEBUG)
    _owner = pthread_self();
#endif
  }

  inline void Release() {
#if !defined(NDEBUG)
    assert(pthread_equal(_owner, pthread_self()) != 0);
#endif

    __sync_add_and_fetch(reinterpret_cast<int*>(&_value), 1);
    _owner = 0;
  }

  inline bool TryAcquire(unsigned long timeout = 0) {
    bool wasLocked = __sync_sub_and_fetch(reinterpret_cast<int*>(&_value), 1) == 0;
    if (!wasLocked) __sync_add_and_fetch(reinterpret_cast<int*>(&_value), 1);

#if !defined(NDEBUG)
    if (wasLocked) _owner = pthread_self();
#endif

    return wasLocked;
  }

}; /* FastLock */

}  // namespace ZThread

#endif

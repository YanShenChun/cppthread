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

#ifndef __ZTFAIRREADWRITELOCK_H__
#define __ZTFAIRREADWRITELOCK_H__

#include "zthread/condition.h"
#include "zthread/guard.h"
#include "zthread/mutex.h"
#include "zthread/read_write_lock.h"

namespace zthread {

/**
 * @class FairReadWriteLock
 *
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T10:26:25-0400>
 * @version 2.2.7
 *
 * A FairReadWriteLock maintains a balance between the order read-only access
 * and read-write access is allowed. Threads contending for the pair of Lockable
 * objects this ReadWriteLock provides will gain access to the locks in FIFO
 * order.
 *
 * @see ReadWriteLock
 */
class FairReadWriteLock : public ReadWriteLock {
  Mutex lock_;
  Condition cond_;

  volatile int readers_;

  //! @class ReadLock
  class ReadLock : public Lockable {
    FairReadWriteLock& rwlock_;

   public:
    ReadLock(FairReadWriteLock& rwlock) : rwlock_(rwlock) {}

    virtual ~ReadLock() {}

    virtual void Acquire() {
      Guard<Mutex> g(rwlock_.lock_);
      ++rwlock_.readers_;
    }

    virtual bool TryAcquire(unsigned long timeout) {
      if (!rwlock_.lock_.TryAcquire(timeout)) return false;

      ++rwlock_.readers_;
      rwlock_.lock_.Release();

      return true;
    }

    virtual void Release() {
      Guard<Mutex> g(rwlock_.lock_);
      --rwlock_.readers_;

      if (rwlock_.readers_ == 0) rwlock_.cond_.Signal();
    }
  };

  //! @class WriteLock
  class WriteLock : public Lockable {
    FairReadWriteLock& rwlock_;

   public:
    WriteLock(FairReadWriteLock& rwlock) : rwlock_(rwlock) {}

    virtual ~WriteLock() {}

    virtual void Acquire() {
      rwlock_.lock_.Acquire();

      try {
        while (rwlock_.readers_ > 0) rwlock_.cond_.Wait();
      } catch (...) {
        rwlock_.lock_.Release();
        throw;
      }
    }

    virtual bool TryAcquire(unsigned long timeout) {
      if (!rwlock_.lock_.TryAcquire(timeout)) return false;

      try {
        while (rwlock_.readers_ > 0) rwlock_.cond_.Wait(timeout);
      } catch (...) {
        rwlock_.lock_.Release();
        throw;
      }

      return true;
    }

    virtual void Release() { rwlock_.lock_.Release(); }
  };

  friend class ReadLock;
  friend class WriteLock;

  ReadLock rlock_;
  WriteLock wlock_;

 public:
  /**
   * Create a BiasedReadWriteLock
   *
   * @exception Initialization_Exception thrown if resources could not be
   *            allocated for this object.
   */
  FairReadWriteLock()
      : cond_(lock_), readers_(0), rlock_(*this), wlock_(*this) {}

  //! Destroy this ReadWriteLock
  virtual ~FairReadWriteLock() {}

  /**
   * @see ReadWriteLock::getReadLock()
   */
  virtual Lockable& GetReadLock() { return rlock_; }

  /**
   * @see ReadWriteLock::getWriteLock()
   */
  virtual Lockable& GetWriteLock() { return wlock_; }
};

}; // namespace zthread

#endif // __ZTFAIRREADWRITELOCK_H__

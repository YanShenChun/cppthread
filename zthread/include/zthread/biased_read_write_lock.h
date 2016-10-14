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

#ifndef __ZTBIASEDREADWRITELOCK_H__
#define __ZTBIASEDREADWRITELOCK_H__

#include "zthread/condition.h"
#include "zthread/fast_mutex.h"
#include "zthread/guard.h"
#include "zthread/read_write_lock.h"

namespace zthread {

/**
 * @class BiasedReadWriteLock
 *
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T10:22:34-0400>
 * @version 2.2.7
 *
 * A BiasedReadWriteLock has a bias toward writers. It will prefer read-write
 * access over read-only access when many threads are contending for access to
 * either Lockable this ReadWriteLock provides.
 *
 * @see ReadWriteLock
 */
class BiasedReadWriteLock : public ReadWriteLock {
  FastMutex lock_;
  Condition cond_read_;
  Condition cond_write_;

  volatile int active_writers_;
  volatile int active_readers_;

  volatile int waiting_readers_;
  volatile int waiting_writers_;

  //! @class ReadLock
  class ReadLock : public Lockable {
    BiasedReadWriteLock& rwlock_;

   public:
    ReadLock(BiasedReadWriteLock& rwlock) : rwlock_(rwlock) {}

    virtual ~ReadLock() {}

    virtual void Acquire() { rwlock_.BeforeRead(); }

    virtual bool TryAcquire(unsigned long timeout) {
      return rwlock_.BeforeReadAttempt(timeout);
    }

    virtual void Release() { rwlock_.AfterRead(); }
  };

  //! @class WriteLock
  class WriteLock : public Lockable {
    BiasedReadWriteLock& rwlock_;

   public:
    WriteLock(BiasedReadWriteLock& rwlock) : rwlock_(rwlock) {}

    virtual ~WriteLock() {}

    virtual void Acquire() { rwlock_.BeforeWrite(); }

    virtual bool TryAcquire(unsigned long timeout) {
      return rwlock_.BeforeWriteAttempt(timeout);
    }

    virtual void Release() { rwlock_.AfterWrite(); }
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
  BiasedReadWriteLock()
      : cond_read_(lock_), cond_write_(lock_), rlock_(*this), wlock_(*this) {
    active_writers_ = 0;
    active_readers_ = 0;

    waiting_readers_ = 0;
    waiting_writers_ = 0;
  }

  //! Destroy this ReadWriteLock
  virtual ~BiasedReadWriteLock() {}

  /**
   * @see ReadWriteLock::GetReadLock()
   */
  virtual Lockable& GetReadLock() { return rlock_; }

  /**
   * @see ReadWriteLock::GetWriteLock()
   */
  virtual Lockable& GetWriteLock() { return wlock_; }

 protected:
  void BeforeRead() {
    Guard<FastMutex> guard(lock_);

    ++waiting_readers_;

    while (!AllowReader()) {
      try {
        cond_read_.Wait();
      } catch (...) {
        --waiting_readers_;
        throw;
      }
    }

    --waiting_readers_;
    ++active_readers_;
  }

  bool BeforeReadAttempt(unsigned long timeout) {
    Guard<FastMutex> guard(lock_);
    bool result = false;

    ++waiting_readers_;

    while (!AllowReader()) {
      try {
        result = cond_read_.Wait(timeout);
      } catch (...) {
        --waiting_readers_;
        throw;
      }
    }

    --waiting_readers_;
    ++active_readers_;

    return result;
  }

  void AfterRead() {
    bool wake_reader = false;
    bool wake_writer = false;

    {
      Guard<FastMutex> guard(lock_);

      --active_readers_;

      wake_reader = (waiting_readers_ > 0);
      wake_writer = (waiting_writers_ > 0);
    }

    if (wake_writer)
      cond_write_.Signal();
    else if (wake_reader)
      cond_read_.Signal();
  }

  void BeforeWrite() {
    Guard<FastMutex> guard(lock_);

    ++waiting_writers_;

    while (!AllowWriter()) {
      try {
        cond_write_.Wait();
      } catch (...) {
        --waiting_writers_;
        throw;
      }
    }

    --waiting_writers_;
    ++active_writers_;
  }

  bool BeforeWriteAttempt(unsigned long timeout) {
    Guard<FastMutex> guard(lock_);
    bool result = false;

    ++waiting_writers_;

    while (!AllowWriter()) {
      try {
        result = cond_write_.Wait(timeout);
      } catch (...) {
        --waiting_writers_;
        throw;
      }
    }

    --waiting_writers_;
    ++active_writers_;

    return result;
  }

  void AfterWrite() {
    bool wake_reader = false;
    bool wake_writer = false;

    {
      Guard<FastMutex> guard(lock_);

      --active_writers_;

      wake_reader = (waiting_readers_ > 0);
      wake_writer = (waiting_writers_ > 0);
    }

    if (wake_writer)
      cond_write_.Signal();
    else if (wake_reader)
      cond_read_.Signal();
  }

  bool AllowReader() { return (active_writers_ == 0); }

  bool AllowWriter() { return (active_writers_ == 0 && active_readers_ == 0); }
};

}; // namespace zthread

#endif // __ZTBIASEDREADWRITELOCK_H__

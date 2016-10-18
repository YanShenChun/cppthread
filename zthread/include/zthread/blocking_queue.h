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

#ifndef __ZTBLOCKINGQUEUE_H__
#define __ZTBLOCKINGQUEUE_H__

#include "zthread/condition.h"
#include "zthread/guard.h"
#include "zthread/queue.h"

#include <deque>

namespace zthread {

/**
 * @class BlockingQueue
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T12:01:43-0400>
 * @version 2.3.0
 *
 * Like a LockedQueue, a BlockingQueue is a Queue implementation that provides
 * serialized access to the items added to it. It differs by causing threads
 * accessing the Next() methods to block until a value becomes available.
 */
template <class T, class LockType, typename StorageType = std::deque<T> >
class BlockingQueue : public Queue<T>, public Lockable {
  //! Serialize access
  LockType lock_;

  //! Signaled when empty
  Condition not_empty_;

  //! Storage backing the queue
  StorageType queue_;

  //! Cancellation flag
  volatile bool canceled_;

 public:
  //! Create a new BlockingQueue
  BlockingQueue() : not_empty_(lock_), canceled_(false) {}

  //! Destroy this BlockingQueue
  virtual ~BlockingQueue() {}

  /**
   * @see Queue::Add(const T& item)
   */
  virtual void Add(const T& item) {
    Guard<LockType> g(lock_);

    if (canceled_) throw CancellationException();

    queue_.push_back(item);

    not_empty_.Signal();
  }

  /**
   * @see Queue::Add(const T& item, unsigned long timeout)
   */
  virtual bool Add(const T& item, unsigned long timeout) {
    try {
      Guard<LockType> g(lock_, timeout);

      if (canceled_) throw CancellationException();

      queue_.push_back(item);

      not_empty_.Signal();
    } catch (TimeoutException&) {
      return false;
    }

    return true;
  }

  /**
   * Get a value from this Queue. The calling thread may block indefinitely.
   *
   * @return <em>T</em> next available value
   *
   * @exception Cancellation_Exception thrown if this Queue has been canceled.
   *
   * @exception Interrupted_Exception thrown if the calling thread is
   * interrupted before a value becomes available.
   *
   * @pre The Queue should not have been canceled prior to the invocation of
   * this function.
   * @post The value returned will have been removed from the Queue.
   *
   * @see Queue::Next()
   */
  virtual T Next() {
    Guard<LockType> g(lock_);

    while (queue_.size() == 0 && !canceled_) not_empty_.Wait();

    if (queue_.size() == 0) throw CancellationException();

    T item = queue_.front();
    queue_.pop_front();

    return item;
  }

  /**
   * Get a value from this Queue. The calling thread may block indefinitely.
   *
   * @param timeout maximum amount of time (milliseconds) this method may block
   *        the calling thread.
   *
   * @return <em>T</em> next available value
   *
   * @exception Cancellation_Exception thrown if this Queue has been canceled.
   *
   * @exception Timeout_Exception thrown if the timeout expires before a value
   *            can be retrieved.
   *
   * @exception Interrupted_Exception thrown if the calling thread is
   * interrupted before a value becomes available.
   *
   * @pre  The Queue should not have been canceled prior to the invocation of
   * this function.
   * @post The value returned will have been removed from the Queue.
   *
   * @see Queue::Next(unsigned long timeout)
   */
  virtual T Next(unsigned long timeout) {
    Guard<LockType> g(lock_, timeout);

    while (queue_.size() == 0 && !canceled_) {
      if (!not_empty_.Wait(timeout)) throw TimeoutException();
    }

    if (queue_.size() == 0) throw CancellationException();

    T item = queue_.front();
    queue_.pop_front();

    return item;
  }

  /**
   * @see Queue::Cancel()
   *
   * @post If threads are blocked on one of the Next() functions then
   *       they will be awakened with a Cancellation_Exception.
   */
  virtual void Cancel() {
    Guard<LockType> g(lock_);

    not_empty_.Broadcast();
    canceled_ = true;
  }

  /**
   * @see Queue::IsCanceled()
   */
  virtual bool IsCanceled() {
    // Faster check since the queue will not become un-canceled
    if (canceled_) return true;

    Guard<LockType> g(lock_);

    return canceled_;
  }

  /**
   * @see Queue::Size()
   */
  virtual size_t Size() {
    Guard<LockType> g(lock_);
    return queue_.size();
  }

  /**
   * @see Queue::Size(unsigned long timeout)
   */
  virtual size_t Size(unsigned long timeout) {
    Guard<LockType> g(lock_, timeout);
    return queue_.size();
  }

 public:
  virtual void Acquire() { lock_.Acquire(); }

  virtual bool TryAcquire(unsigned long timeout) {
    return lock_.TryAcquire(timeout);
  }

  virtual void Release() { lock_.Release(); }

}; /* BlockingQueue */

}  // namespace zthread

#endif  // __ZTBLOCKINGQUEUE_H__

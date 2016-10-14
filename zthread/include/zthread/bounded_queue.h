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

#ifndef __ZTBOUNDEDQUEUE_H__
#define __ZTBOUNDEDQUEUE_H__

#include "zthread/condition.h"
#include "zthread/guard.h"
#include "zthread/queue.h"

#include <deque>

namespace zthread {

/**
 * @class BoundedQueue
 *
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T13:54:04-0400>
 * @version 2.3.0
 *
 * A BoundedQueue provides serialized access to a set of values. It differs from
 * other Queues by adding a maximum capacity, giving it the following 
 * properties:
 *
 * - Threads calling the empty() methods will be blocked until the BoundedQueue
 *   becomes empty.
 * - Threads calling the next() methods will be blocked until the BoundedQueue
 *   has a value to return.
 * - Threads calling the add() methods will be blocked until the number of
 *   values in the Queue drops below the maximum capacity.
 *
 * @see Queue
 */
template <class T, class LockType, typename StorageType = std::deque<T> >
class BoundedQueue : public Queue<T>, public Lockable {
  //! Maximum capacity for the Queue
  size_t capacity_;

  //! Serialize access
  LockType lock_;

  //! Signaled if not full
  Condition not_full_;

  //! Signaled if not empty
  Condition not_empty_;

  //! Signaled if empty
  Condition is_empty_;

  //! Storage backing the queue
  StorageType queue_;

  //! Cancellation flag
  volatile bool canceled_;

 public:
  /**
   * Create a BoundedQueue with the given capacity.
   *
   * @param capacity maximum number of values to allow in the Queue at
   *        at any time
   */
  BoundedQueue(size_t capacity)
      : not_full_(lock_),
        not_empty_(lock_),
        is_empty_(lock_),
        capacity_(capacity),
        canceled_(false) {}

  //! Destroy this Queue
  virtual ~BoundedQueue() {}

  /**
   * Get the maximum capacity of this Queue.
   *
   * @return <i>size_t</i> maximum capacity
   */
  size_t capacity() { return capacity_; }

  /**
   * Add a value to this Queue.
   *
   * If the number of values in the queue matches the value returned by
   * <i>capacity</i>()
   * then the calling thread will be blocked until at least one value is removed
   * from the Queue.
   *
   * @param item value to be added to the Queue
   *
   * @exception CancellationException thrown if this Queue has been canceled.
   * @exception InterruptedException thrown if the thread was interrupted while
   * waiting to add a value
   *
   * @pre  The Queue should not have been canceled prior to the invocation of
   *       this function.
   * @post If no exception is thrown, a copy of <i>item</i> will have been added
   *       to the Queue.
   *
   * @see Queue::add(const T& item)
   */
  virtual void Add(const T& item) {
    Guard<LockType> g(lock_);

    // Wait for the capacity of the Queue to drop
    while ((queue_.size() == capacity_) && !canceled_) not_full_.Wait();

    if (canceled_) throw Cancellation_Exception();

    queue_.push_back(item);
    not_empty_.signal();  // Wake any waiters
  }

  /**
   * Add a value to this Queue.
   *
   * If the number of values in the queue matches the value returned by
   * <i>capacity</i>() then the calling thread will be blocked until at least 
   * one value is removed from the Queue.
   *
   * @param item value to be added to the Queue
   * @param timeout maximum amount of time (milliseconds) this method may block
   *        the calling thread.
   *
   * @return
   *   - <em>true</em> if a copy of <i>item</i> can be added before <i>timeout
   *   </i> milliseconds elapse.
   *   - <em>false</em> otherwise.
   *
   * @exception CancellationException thrown if this Queue has been canceled.
   * @exception InterruptedException thrown if the thread was interrupted while
   * waiting to add a value
   *
   * @pre  The Queue should not have been canceled prior to the invocation of
   * this function.
   * @post If no exception is thrown, a copy of <i>item</i> will have been added
   * to the Queue.
   *
   * @see Queue::add(const T& item, unsigned long timeout)
   */
  virtual bool Add(const T& item, unsigned long timeout) {
    try {
      Guard<LockType> g(lock_, timeout);

      // Wait for the capacity of the Queue to drop
      while ((queue_.size() == capacity_) && !canceled_)
        if (!not_full_.Wait(timeout)) return false;

      if (canceled_) throw Cancellation_Exception();

      queue_.push_back(item);
      not_empty_.signal();  // Wake any waiters
    } catch (Timeout_Exception&) {
      return false;
    }

    return true;
  }

  /**
   * Retrieve and remove a value from this Queue.
   *
   * If invoked when there are no values present to return then the calling
   * thread will be blocked until a value arrives in the Queue.
   *
   * @return <em>T</em> next available value
   *
   * @exception CancellationException thrown if this Queue has been canceled.
   * @exception InterruptedException thrown if the thread was interrupted while
   * waiting to retrieve a value
   *
   * @pre  The Queue should not have been canceled prior to the invocation of
   * this function.
   * @post The value returned will have been removed from the Queue.
   */
  virtual T Next() {
    Guard<LockType> g(lock_);

    while (queue_.size() == 0 && !canceled_) not_empty_.Wait();

    if (queue_.size() == 0)  // Queue canceled
      throw Cancellation_Exception();

    T item = queue_.front();
    queue_.pop_front();

    not_full_.signal();  // Wake any thread trying to add

    if (queue_.size() == 0)  // Wake empty waiters
      is_empty_.broadcast();

    return item;
  }

  /**
   * Retrieve and remove a value from this Queue.
   *
   * If invoked when there are no values present to return then the calling
   * thread will be blocked until a value arrives in the Queue.
   *
   * @param timeout maximum amount of time (milliseconds) this method may block
   *        the calling thread.
   *
   * @return <em>T</em> next available value
   *
   * @exception CancellationException thrown if this Queue has been canceled.
   * @exception TimeoutException thrown if the timeout expires before a value
   *            can be retrieved.
   *
   * @pre  The Queue should not have been canceled prior to the invocation of
   * this function.
   * @post The value returned will have been removed from the Queue.
   */
  virtual T Next(unsigned long timeout) {
    Guard<LockType> g(lock_, timeout);

    // Wait for items to be added
    while (queue_.size() == 0 && !canceled_) {
      if (!not_empty_.Wait(timeout)) throw Timeout_Exception();
    }

    if (queue_.size() == 0)  // Queue canceled
      throw Cancellation_Exception();

    T item = queue_.front();
    queue_.pop_front();

    not_full_.signal();  // Wake add() waiters

    if (queue_.size() == 0)  // Wake empty() waiters
      is_empty_.broadcast();

    return item;
  }

  /**
   * Cancel this queue.
   *
   * @post Any threads blocked by an add() function will throw a
   * Cancellation_Exception.
   * @post Any threads blocked by a next() function will throw a
   * Cancellation_Exception.
   *
   * @see Queue::cancel()
   */
  virtual void Cancel() {
    Guard<LockType> g(lock_);

    canceled_ = true;
    not_empty_.broadcast();  // Wake next() waiters
  }

  /**
   * @see Queue::isCanceled()
   */
  virtual bool IsCanceled() {
    // Faster check since the Queue will not become un-canceled
    if (canceled_) return true;

    Guard<LockType> g(lock_);

    return canceled_;
  }

  /**
   * @see Queue::size()
   */
  virtual size_t Size() {
    Guard<LockType> g(lock_);
    return queue_.size();
  }

  /**
   * @see Queue::size(unsigned long timeout)
   */
  virtual size_t Size(unsigned long timeout) {
    Guard<LockType> g(lock_, timeout);
    return queue_.size();
  }

  /**
   * Test whether any values are available in this Queue.
   *
   * The calling thread is blocked until there are no values present
   * in the Queue.
   *
   * @return
   *  - <em>true</em> if there are no values available.
   *  - <em>false</em> if there <i>are</i> values available.
   *
   * @see Queue::empty()
   */
  virtual bool Empty() {
    Guard<LockType> g(lock_);

    while (queue_.size() > 0)  // Wait for an empty signal
      is_empty_.Wait();

    return true;
  }

  /**
   * Test whether any values are available in this Queue.
   *
   * The calling thread is blocked until there are no values present
   * in the Queue.
   *
   * @param timeout maximum amount of time (milliseconds) this method may block
   *        the calling thread.
   *
   * @return
   *  - <em>true</em> if there are no values available.
   *  - <em>false</em> if there <i>are</i> values available.
   *
   * @exception TimeoutException thrown if <i>timeout</i> milliseconds
   *            expire before a value becomes available
   *
   * @see Queue::empty()
   */
  virtual bool Empty(unsigned long timeout) {
    Guard<LockType> g(lock_, timeout);

    while (queue_.size() > 0)  // Wait for an empty signal
      is_empty_.Wait(timeout);

    return true;
  }

 public:
  virtual void Acquire() { lock_.Acquire(); }

  virtual bool TryAcquire(unsigned long timeout) {
    return lock_.TryAcquire(timeout);
  }

  virtual void Release() { lock_.Release(); }

}; /* BoundedQueue */

}  // namespace zthread

#endif  // __ZTBOUNDEDQUEUE_H__

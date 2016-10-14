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

#ifndef __ZTMONITOREDQUEUE_H__
#define __ZTMONITOREDQUEUE_H__

#include "zthread/condition.h"
#include "zthread/guard.h"
#include "zthread/queue.h"

#include <deque>

namespace zthread {

/**
 * @class MonitoredQueue
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T20:23:28-0400>
 * @version 2.3.0
 *
 * A MonitoredQueue is a Queue implementation that provides  serialized access
 * to the
 * items added to it.
 *
 * - Threads calling the empty() methods will be blocked until the BoundedQueue
 * becomes empty.
 * - Threads calling the next() methods will be blocked until the BoundedQueue
 * has a value to
 *   return.
 *
 * @see Queue
 */
template <class T, class LockType, typename StorageType = std::deque<T> >
class MonitoredQueue : public Queue<T>, public Lockable {
  //! Serialize access
  LockType _lock;

  //! Signaled on not empty
  Condition _notEmpty;

  //! Signaled on empty
  Condition _isEmpty;

  //! Storage backing the queue
  StorageType _queue;

  //! Cancellation flag
  volatile bool _canceled;

 public:
  //! Create a new MonitoredQueue
  MonitoredQueue() : _notEmpty(_lock), _isEmpty(_lock), _canceled(false) {}

  //! Destroy a MonitoredQueue, delete remaining items
  virtual ~MonitoredQueue() {}

  /**
   * Add a value to this Queue.
   *
   * @param item value to be added to the Queue
   *
   * @exception Cancellation_Exception thrown if this Queue has been canceled.
   * @exception Interrupted_Exception thrown if the thread was interrupted while
   * waiting
   *            to add a value
   *
   * @pre  The Queue should not have been canceled prior to the invocation of
   * this function.
   * @post If no exception is thrown, a copy of <i>item</i> will have been added
   * to the Queue.
   *
   * @see Queue::add(const T& item)
   */
  virtual void Add(const T& item) {
    Guard<LockType> g(_lock);

    // Allow no further additions in the canceled state
    if (_canceled) throw Cancellation_Exception();

    _queue.push_back(item);

    _notEmpty.Signal();  // Wake one waiter
  }

  /**
   * Add a value to this Queue.
   *
   * @param item value to be added to the Queue
   * @param timeout maximum amount of time (milliseconds) this method may block
   *        the calling thread.
   *
   * @return
   *   - <em>true</em> if a copy of <i>item</i> can be added before
   * <i>timeout</i>
   *     milliseconds elapse.
   *   - <em>false</em> otherwise.
   *
   * @exception Cancellation_Exception thrown if this Queue has been canceled.
   * @exception Interrupted_Exception thrown if the thread was interrupted while
   * waiting
   *            to add a value
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
      Guard<LockType> g(_lock, timeout);

      if (_canceled) throw Cancellation_Exception();

      _queue.push_back(item);

      _notEmpty.Signal();

    } catch (Timeout_Exception&) {
      return false;
    }

    return true;
  }

  /**
   * Retrieve and remove a value from this Queue.
   *
   * If invoked when there are no values present to return then the calling
   * thread
   * will be blocked until a value arrives in the Queue.
   *
   * @return <em>T</em> next available value
   *
   * @exception Cancellation_Exception thrown if this Queue has been canceled.
   * @exception Interrupted_Exception thrown if the thread was interrupted while
   * waiting
   *            to retrieve a value
   *
   * @pre  The Queue should not have been canceled prior to the invocation of
   * this function.
   * @post The value returned will have been removed from the Queue.
   */
  virtual T Next() {
    Guard<LockType> g(_lock);

    while (_queue.size() == 0 && !_canceled) _notEmpty.Wait();

    if (_queue.size() == 0)  // Queue canceled
      throw Cancellation_Exception();

    T item = _queue.front();
    _queue.pop_front();

    if (_queue.size() == 0)  // Wake empty waiters
      _isEmpty.Broadcast();

    return item;
  }

  /**
   * Retrieve and remove a value from this Queue.
   *
   * If invoked when there are no values present to return then the calling
   * thread
   * will be blocked until a value arrives in the Queue.
   *
   * @param timeout maximum amount of time (milliseconds) this method may block
   *        the calling thread.
   *
   * @return <em>T</em> next available value
   *
   * @exception Cancellation_Exception thrown if this Queue has been canceled.
   * @exception Timeout_Exception thrown if the timeout expires before a value
   *            can be retrieved.
   *
   * @pre  The Queue should not have been canceled prior to the invocation of
   * this function.
   * @post The value returned will have been removed from the Queue.
   */
  virtual T Next(unsigned long timeout) {
    Guard<LockType> g(_lock, timeout);

    while (_queue.size() == 0 && !_canceled) {
      if (!_notEmpty.Wait(timeout)) throw Timeout_Exception();
    }

    if (_queue.size() == 0)  // Queue canceled
      throw Cancellation_Exception();

    T item = _queue.front();
    _queue.pop_front();

    if (_queue.size() == 0)  // Wake empty waiters
      _isEmpty.Broadcast();

    return item;
  }

  /**
   * Cancel this queue.
   *
   * @post Any threads blocked by a next() function will throw a
   * Cancellation_Exception.
   *
   * @see Queue::cancel()
   */
  virtual void Cancel() {
    Guard<LockType> g(_lock);

    _canceled = true;
    _notEmpty.Broadcast();  // Wake next() waiters
  }

  /**
   * @see Queue::isCanceled()
   */
  virtual bool IsCanceled() {
    // Faster check since the queue will not become un-canceled
    if (_canceled) return true;

    Guard<LockType> g(_lock);

    return _canceled;
  }

  /**
   * @see Queue::size()
   */
  virtual size_t Size() {
    Guard<LockType> g(_lock);
    return _queue.size();
  }

  /**
   * @see Queue::size(unsigned long timeout)
   */
  virtual size_t Size(unsigned long timeout) {
    Guard<LockType> g(_lock, timeout);
    return _queue.size();
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
  virtual bool empty() {
    Guard<LockType> g(_lock);

    while (_queue.size() > 0)  // Wait for an empty signal
      _isEmpty.Wait();

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
   * @exception Timeout_Exception thrown if <i>timeout</i> milliseconds
   *            expire before a value becomes available
   *
   * @see Queue::empty()
   */
  virtual bool empty(unsigned long timeout) {
    Guard<LockType> g(_lock, timeout);

    while (_queue.size() > 0)  // Wait for an empty signal
      _isEmpty.Wait(timeout);

    return true;
  }

 public:
  virtual void Acquire() { _lock.Acquire(); }

  virtual bool TryAcquire(unsigned long timeout) {
    return _lock.TryAcquire(timeout);
  }

  virtual void Release() { _lock.Release(); }

}; /* MonitoredQueue */

}  // namespace ZThread

#endif  // __ZTMONITOREDQUEUE_H__

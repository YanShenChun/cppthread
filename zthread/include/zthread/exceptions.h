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

#ifndef __ZTEXCEPTIONS_H__
#define __ZTEXCEPTIONS_H__

#include <string>
#include "zthread/config.h"

namespace zthread {

/**
 * @class SynchronizationException
 *
 * Serves as a general base class for the Exception hierarchy used within
 * this package.
 *
 */
class SynchronizationException {
  // Restrict heap allocation
  static void* operator new(size_t size);
  static void* operator new[](size_t size);

  std::string msg_;

 public:
  /**
   * Create a new exception with a default error message 'Synchronization
   * Exception'
   */
  SynchronizationException() : msg_("Synchronization exception") {}

  /**
   * Create a new exception with a given error message
   *
   * @param const char* - error message
   */
  SynchronizationException(const char* msg) : msg_(msg) {}

  /**
   * Get additional info about the exception
   *
   * @return const char* for the error message
   */
  const char* what() const { return msg_.c_str(); }
};

/**
 * @class InterruptedException
 *
 * Used to describe an interrupted operation that would have normally
 * blocked the calling thread
 */
class InterruptedException : public SynchronizationException {
 public:
  //! Create a new exception
  InterruptedException() : SynchronizationException("Thread interrupted") {}

  //! Create a new exception
  InterruptedException(const char* msg) : SynchronizationException(msg) {}
};

/**
 * @class DeadlockException
 *
 * Thrown when deadlock has been detected
 */
class DeadlockException : public SynchronizationException {
 public:
  //! Create a new exception
  DeadlockException() : SynchronizationException("Deadlock detected") {}

  //! Create a new exception
  DeadlockException(const char* msg) : SynchronizationException(msg) {}
};

/**
 * @class InvalidOpException
 *
 * Thrown when performing an illegal operation this object
 */
class InvalidOpException : public SynchronizationException {
 public:
  //! Create a new exception
  InvalidOpException() : SynchronizationException("Invalid operation") {}
  //! Create a new exception
  InvalidOpException(const char* msg) : SynchronizationException(msg) {}
};

/**
 * @class InitializationException
 *
 * Thrown when the system has no more resources to create new
 * synchronization controls
 */
class InitializationException : public SynchronizationException {
 public:
  //! Create a new exception
  InitializationException()
      : SynchronizationException("Initialization error") {}
  //! Create a new exception
  InitializationException(const char* msg) : SynchronizationException(msg) {}
};

/**
 * @class CancellationException
 *
 * Cancellation_Exceptions are thrown by 'Canceled' objects.
 * @see Cancelable
 */
class CancellationException : public SynchronizationException {
 public:
  //! Create a new Cancelltion_Exception
  CancellationException() : SynchronizationException("Canceled") {}
  //! Create a new Cancelltion_Exception
  CancellationException(const char* msg) : SynchronizationException(msg) {}
};

/**
 * @class TimeoutException
 *
 * There is no need for error messaged simply indicates the last
 * operation timed out
 */
class TimeoutException : public SynchronizationException {
 public:
  //! Create a new Timeout_Exception
  TimeoutException() : SynchronizationException("Timeout") {}
  //! Create a new
  TimeoutException(const char* msg) : SynchronizationException(msg) {}
};

/**
 * @class NoSuchElementException
 *
 * The last operation that was attempted on a Queue could not find
 * the item that was indicated (during that last Queue method invocation)
 */
class NoSuchElementException {
 public:
  //! Create a new exception
  NoSuchElementException() {}
};

/**
 * @class InvalidTaskException
 *
 * Thrown when a task is not valid (e.g. null or start()ing a thread with
 * no overriden run() method)
 */
class InvalidTaskException : public InvalidOpException {
 public:
  //! Create a new exception
  InvalidTaskException() : InvalidOpException("Invalid task") {}
};

/**
 * @class BrokenBarrierException
 *
 * Thrown when a Barrier is broken because one of the participating threads
 * has been interrupted.
 */
class BrokenBarrierException : public SynchronizationException {
 public:
  //! Create a new exception
  BrokenBarrierException() : SynchronizationException("Barrier broken") {}

  //! Create a new exception
  BrokenBarrierException(const char* msg) : SynchronizationException(msg) {}
};

/**
 * @class FutureException
 *
 * Thrown when there is an error using a Future.
 */
class FutureException : public SynchronizationException {
 public:
  //! Create a new exception
  FutureException() : SynchronizationException() {}

  //! Create a new exception
  FutureException(const char* msg) : SynchronizationException(msg) {}
};
}; // namespace zthread

#endif  // __ZTEXCEPTIONS_H__

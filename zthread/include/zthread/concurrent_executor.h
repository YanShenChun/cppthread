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

#ifndef __ZTCONCURRENTEXECUTOR_H__
#define __ZTCONCURRENTEXECUTOR_H__

#include "zthread/pool_executor.h"

namespace zthread {

/**
 * @class ConcurrentExecutor
 *
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T22:36:11-0400>
 * @version 2.3.0
 *
 * A ConcurrentExecutor spawns a single thread to service a series of Tasks.
 *
 * @see PoolExecutor.
 */
class ConcurrentExecutor : public Executor {
  PoolExecutor executor_;

 public:
  //! Create a ConcurrentExecutor
  ConcurrentExecutor();

  /**
   * Interrupting a ConcurrentExecutor will cause the thread running the tasks
   * to be interrupted once during the execution of each task that has been
   * submitted at the time this function is called.
   *
   * Tasks that are submitted after this function is called will
   * not be interrupt()ed; unless this function is invoked again().
   *
   * @code
   *
   * void aFunction() {
   *
   *   ConcurrentExecutor executor;
   *
   *   // Submit p Tasks
   *   for(size_t n = 0; n < p; n++)
   *     executor.Execute(new aRunnable);
   *
   *   // Tasks [m, p) may be interrupted, where m is the first task that has
   *   // not completed at the time the interrupt() is invoked.
   *   executor.Interrupt();
   *
   *   // Submit (q - p) Tasks
   *   for(size_t n = p; n < q; n++)
   *     executor.Execute(new Chore);
   *
   *   // Tasks [p, q) are not interrupted
   *
   * }
   *
   * @endcode
   */
  virtual void Interrupt();

  /**
   * Submit a Task to this Executor. This will not block the current thread
   * for very long. The task will be enqueued internally and eventually run
   * in the context of the single thread driving all the Tasks submitted to this
   * Executor.
   *
   * @exception CancellationException thrown if this Executor has been
   * canceled.
   * The Task being submitted will not be executed by this Executor.
   *
   * @exception SynchronizationException thrown only in the event of an error
   * in the implementation of the library.
   *
   * @see Executor::Execute(const Task&)
   */
  virtual void Execute(const Task&);

  /**
   * @see Cancelable::Cancel()
   */
  virtual void Cancel();

  /**
   * @see Cancelable::isCanceled()
   */
  virtual bool IsCanceled();

  /**
   * @see PoolExecutor::wait()
   */
  virtual void Wait();

  /**
   * @see PoolExecutor::Wait(unsigned long timeout)
   */
  virtual bool Wait(unsigned long timeout);
}; /* ConcurrentExecutor */

}  // namespace zthread

#endif  // __ZTCONCURRENTEXECUTOR_H__

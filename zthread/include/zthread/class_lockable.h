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

#ifndef __ZTCLASSLOCKABLE_H__
#define __ZTCLASSLOCKABLE_H__

#include "zthread/counted_ptr.h"
#include "zthread/mutex.h"

namespace zthread {

/**
 * @class ClassLockable
 *
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T23:37:38-0400>
 * @version 2.3.0
 *
 *
 */
template <typename ClassType, class LockType = Mutex>
class ClassLockable : public Lockable {
  static CountedPtr<LockType> instance_;
  CountedPtr<LockType> lock_;

 public:
  //! Create a ClassLockable
  ClassLockable() : lock_(instance_) {}

  //! Acquire() the ClassLockable
  virtual void Acquire() { lock_->Acquire(); }

  //! TryAcquire() the ClassLockable
  virtual bool TryAcquire(unsigned long timeout) {
    return lock_->TryAcquire(timeout);
  }

  //! Release() the ClassLockable
  virtual void Release() { lock_->Release(); }
};

template <typename ClassType, class LockType>
CountedPtr<LockType> ClassLockable<ClassType, LockType>::instance_(
    new LockType);

}  // namespace zthread

#endif  // __ZTCLASSLOCKABLE_H__

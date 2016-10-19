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

#ifndef __ZTGUARD_H__
#define __ZTGUARD_H__

#include "zthread/exceptions.h"
#include "zthread/non_copyable.h"

namespace zthread {

//
// GuardLockingPolicyContract {
//
// CreateScope(lock_type&)
// bool CreateScope(lock_type&, unsigned long)
// DestroyScope(lock_type&)
//
// }
//

/**
 * @class LockHolder
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T17:55:42-0400>
 * @version 2.2.0
 *
 * This is a simple base class for Guards class. It allows Guards
 * that have compatible targets to refer to each others targets
 * allowing for the construction of Guards that share the same lock
 * but have different locking policies.
 */
template <class LockType>
class LockHolder {
  LockType& lock_;
  bool enabled_;

 public:
  template <class T>
  LockHolder(T& t) : lock_(extract(t).lock_), enabled_(true) {}

  LockHolder(LockHolder& holder) : lock_(holder.lock_), enabled_(true) {}

  LockHolder(LockType& lock) : lock_(lock), enabled_(true) {}

  void Disable() { enabled_ = false; }

  bool IsDisabled() { return !enabled_; }

  LockType& GetLock() { return lock_; }

 protected:
  template <class T>
  static LockHolder& extract(T& t) {
    // Design and Evolution of C++, page 328
    return (LockHolder&)(t);
  }
};

/**
 * @class CompoundScope
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T17:55:42-0400>
 * @version 2.2.0
 *
 * Locking policy that aggregates two policies that share a target.
 * It is not appropriate to use with any type of OverlappedScope
 */
template <class Scope1, class Scope2>
class CompoundScope {
 public:
  template <class LockType>
  static void CreateScope(LockHolder<LockType>& l) {
    Scope1::CreateScope(l);
    Scope2::CreateScope(l);
  }

  template <class LockType>
  static bool CreateScope(LockHolder<LockType>& l, unsigned long ms) {
    if (Scope1::CreateScope(l, ms))
      if (!Scope2::CreateScope(l, ms)) {
        Scope1::DestroyScope(l);
        return false;
      }

    return true;
  }

  template <class LockType>
  static void DestroyScope(LockHolder<LockType>& l) {
    Scope1::DestroyScope(l);
    Scope2::DestroyScope(l);
  }
};

/**
 * @class LockedScope
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T17:55:42-0400>
 * @version 2.2.0
 *
 * Locking policy for Lockable objects. This policy Acquire()s a Lockable
 * when the protection scope is created, and it Release()s a Lockable
 * when the scope is destroyed.
 */
class LockedScope {
 public:
  /**
   * A new protection scope is being created by l2, using an existing scope
   * created by l1.
   *
   * @param lock1 LockType1& is the LockHolder that holds the desired lock
   * @param lock2 LockType1& is the LockHolder that wants to share
  template <class LockType1, class LockType2>
  static void ShareScope(LockHolder<LockType1>& l1, LockHolder<LockType2>& l2) {

    l2.getLock().Acquire();

  }
   */

  /**
   * A new protection scope is being created.
   *
   * @param lock LockType& is a type of LockHolder.
   */
  template <class LockType>
  static bool CreateScope(LockHolder<LockType>& l, unsigned long ms) {
    return l.GetLock().TryAcquire(ms);
  }

  /**
   * A new protection scope is being created.
   *
   * @param lock LockType& is a type of LockHolder.
   */
  template <class LockType>
  static void CreateScope(LockHolder<LockType>& l) {
    l.GetLock().Acquire();
  }

  /**
   * A protection scope is being destroyed.
   *
   * @param lock LockType& is a type of LockHolder.
   */
  template <class LockType>
  static void DestroyScope(LockHolder<LockType>& l) {
    l.GetLock().Release();
  }
};

/**
 * @class UnlockedScope
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T17:55:42-0400>
 * @version 2.2.0
 *
 * Locking policy for Lockable objects. This policy Release()s a Lockable
 * when the protection scope is created, and it Acquire()s a Lockable
 * when the scope is destroyed.
 */
class UnlockedScope {
 public:
  /**
   * A new protection scope is being created by l2, using an existing scope
   * created by l1.
   *
   * @param lock1 LockType1& is the LockHolder that holds the desired lock
   * @param lock2 LockType1& is the LockHolder that wants to share
   */
  template <class LockType1, class LockType2>
  static void ShareScope(LockHolder<LockType1>& l1, LockHolder<LockType2>& l2) {
    l2.GetLock().Release();
  }

  /**
   * A new protection scope is being created.
   *
   * @param lock LockType& is a type of LockHolder.
  template <class LockType>
  static void CreateScope(LockHolder<LockType>& l) {

    l.getLock().Release();

  }
   */

  /**
   * A protection scope is being destroyed.
   *
   * @param lock LockType& is a type of LockHolder.
   */
  template <class LockType>
  static void DestroyScope(LockHolder<LockType>& l) {
    l.GetLock().Acquire();
  }
};

/**
 * @class TimedLockedScope
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T17:55:42-0400>
 * @version 2.2.0
 *
 * Locking policy that attempts to enterScope some resource
 * in a certain amount of time using an tryEnterScope-relase protocol.
 */
template <int TimeOut>
class TimedLockedScope {
 public:
  /**
   * Try to enterScope the given LockHolder.
   *
   * @param lock LockType& is a type of LockHolder.
   */
  template <class LockType1, class LockType2>
  static void ShareScope(LockHolder<LockType1>& l1, LockHolder<LockType2>& l2) {
    if (!l2.GetLock().TryAcquire(TimeOut)) throw TimeoutException();
  }

  template <class LockType>
  static void CreateScope(LockHolder<LockType>& l) {
    if (!l.GetLock().TryAcquire(TimeOut)) throw TimeoutException();
  }

  template <class LockType>
  static void DestroyScope(LockHolder<LockType>& l) {
    l.GetLock().Release();
  }
};

/**
 * @class OverlappedScope
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T17:55:42-0400>
 * @version 2.2.0
 *
 * Locking policy allows the effective scope of two locks to overlap
 * by releasing and disabling one lock before its Guard does so.
 */
class OverlappedScope {
 public:
  template <class LockType1, class LockType2>
  static void TransferScope(LockHolder<LockType1>& l1,
                            LockHolder<LockType2>& l2) {
    l1.GetLock().Acquire();

    l2.GetLock().Release();
    l2.Disable();
  }

  template <class LockType>
  static void DestroyScope(LockHolder<LockType>& l) {
    l.GetLock().Release();
  }
};

/**
 * @class Guard
 * @author Eric Crahen <http://www.code-foo.com>
 * @date <2003-07-16T17:55:42-0400>
 * @version 2.2.0
 *
 * Scoped locking utility. This template class can be given a Lockable
 * synchronization object and can 'Guard' or serialize access to
 * that method.
 *
 * For instance, consider a case in which a class or program have a
 * Mutex object associated with it. Access can be serialized with a
 * Guard as shown below.
 *
 * @code
 *
 * Mutex _mtx;
 * void guarded() {
 *
 *    Guard<Mutex> g(_mtx);
 *
 * }
 *
 * @endcode
 *
 * The Guard will lock the synchronization object when it is created and
 * automatically unlock it when it goes out of scope. This eliminates
 * common mistakes like forgetting to unlock your mutex.
 *
 * An alternative to the above example would be
 *
 * @code
 *
 * void guarded() {
 *
 *     (Guard<Mutex>)(_mtx);
 *
 * }
 *
 * @endcode
 *
 * HOWEVER; using a Guard in this method is dangerous. Depending on your
 * compiler an anonymous variable like this can go out of scope immediately
 * which can result in unexpected behavior. - This is the case with MSVC
 * and was the reason for introducing assertions into the Win32_MutexImpl
 * to track this problem down
 *
 */
template <class LockType, class LockingPolicy = LockedScope>
class Guard : private LockHolder<LockType>, private NonCopyable {
  friend class LockHolder<LockType>;

 public:
  /**
   * Create a Guard that enforces a the effective protection scope
   * throughout the lifetime of the Guard object or until the protection
   * scope is modified by another Guard.
   *
   * @param lock LockType the lock this Guard will use to enforce its
   * protection scope.
   * @post the protection scope may be ended prematurely
   */
  Guard(LockType& lock) : LockHolder<LockType>(lock) {
    LockingPolicy::CreateScope(*this);
  };

  /**
   * Create a Guard that enforces a the effective protection scope
   * throughout the lifetime of the Guard object or until the protection
   * scope is modified by another Guard.
   *
   * @param lock LockType the lock this Guard will use to enforce its
   * protection scope.
   * @post the protection scope may be ended prematurely
   */
  Guard(LockType& lock, unsigned long timeout) : LockHolder<LockType>(lock) {
    if (!LockingPolicy::CreateScope(*this, timeout)) throw TimeoutException();
  };

  /**
   * Create a Guard that shares the effective protection scope
   * from the given Guard to this Guard.
   *
   * @param g Guard<U, V> guard that is currently enabled
   * @param lock LockType the lock this Guard will use to enforce its
   * protection scope.
   */
  template <class U, class V>
  Guard(Guard<U, V>& g) : LockHolder<LockType>(g) {
    LockingPolicy::ShareScope(*this, this->extract(g));
  }

  /**
   * Create a Guard that shares the effective protection scope
   * from the given Guard to this Guard.
   *
   * @param g Guard guard that is currently enabled
   * @param lock LockType the lock this Guard will use to enforce its
   * protection scope.
   */
  Guard(Guard& g) : LockHolder<LockType>(g) {
    LockingPolicy::ShareScope(*this, g);
  }

  /**
   * Create a Guard that transfers the effective protection scope
   * from the given Guard to this Guard.
   *
   * @param g Guard<U, V> guard that is currently enabled
   * @param lock LockType the lock this Guard will use to enforce its
   * protection scope.
   */
  template <class U, class V>
  Guard(Guard<U, V>& g, LockType& lock) : LockHolder<LockType>(lock) {
    LockingPolicy::TransferScope(*this, this->extract(g));
  }

  /**
   * Create a Guard that transfers the effective protection scope
   * from the given Guard to this Guard.
   *
   * @param g Guard guard that is currently enabled
   * @param lock LockType the lock this Guard will use to enforce its
   * protection scope.
   */
  Guard(Guard& g, LockType& lock) : LockHolder<LockType>(lock) {
    LockingPolicy::TransferScope(*this, g);
  }

  /**
   * Unlock a given Lockable object with the destruction of this Guard
   */
  ~Guard() throw();

}; /* Guard */

template <class LockType, class LockingPolicy>
Guard<LockType, LockingPolicy>::~Guard() throw() {
  try {
    if (!this->IsDisabled()) LockingPolicy::DestroyScope(*this);
  } catch (...) { /* ignore */
  }
}
}; // namespace zthread

#endif  // __ZTGUARD_H__

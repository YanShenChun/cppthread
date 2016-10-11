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

#ifndef __ZTATOMICCOUNTIMPL_H__
#define __ZTATOMICCOUNTIMPL_H__

//#include <pthread.h>
#include <assert.h>

namespace zthread {

AtomicCount::AtomicCount() { _value = reinterpret_cast<void*>(new long(0)); }

AtomicCount::~AtomicCount() {
  assert(*reinterpret_cast<long*>(_value) == 0);
  delete reinterpret_cast<long*>(_value);
}

//! Postfix decrement and return the previous value
size_t AtomicCount::operator--(int) {
	return __sync_fetch_and_sub(reinterpret_cast<long*>(_value), 1);
}

//! Postfix increment and return the previous value
size_t AtomicCount::operator++(int) {
	return __sync_fetch_and_add(reinterpret_cast<long*>(_value), 1);
}

//! Prefix decrement and return the current value
size_t AtomicCount::operator--() {
	return __sync_sub_and_fetch(reinterpret_cast<long*>(_value), 1);
}

//! Prefix increment and return the current value
size_t AtomicCount::operator++() {
	return __sync_add_and_fetch(reinterpret_cast<long*>(_value), 1);
}

};

#endif  // __ZTATOMICCOUNTIMPL_H__

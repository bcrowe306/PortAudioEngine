#pragma once

#include "threading/choc_SpinLock.h"

// Simple RAII wrapper for SpinLock
class SpinLockGuard {
public:
  explicit SpinLockGuard(choc::threading::SpinLock &lock) : lock_(lock) {
    lock_.lock();
  }
  ~SpinLockGuard() { lock_.unlock(); }
  SpinLockGuard(const SpinLockGuard &) = delete;
  SpinLockGuard &operator=(const SpinLockGuard &) = delete;

private:
  choc::threading::SpinLock &lock_;
};
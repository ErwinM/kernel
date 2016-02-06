#ifndef INCLUDE_SPINLOCK_H
#define INCLUDE_SPINLOCK_H

// Mutual exclusion lock.
struct spinlock {
  uint32_t locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  uint32_t pcs[10];  // The call stack (an array of program counters)
                     // that locked the lock.
};

#endif

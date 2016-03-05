#ifndef INCLUDE_KERNEL_H
#define INCLUDE_KERNEL_H

#include "spinlock.h"


#define NDIRECT 12

struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE } type;
  int ref; // reference count
  char readable;
  char writable;
  struct pipe *pipe;
  struct inode *ip;
  uint32_t off;
};

// in-memory copy of an inode
struct inode {
  uint32_t dev;           // Device number
  uint32_t inum;          // Inode number
  int ref;        		    // Reference count
  int flags;          		// I_BUSY, I_VALID

  uint16_t type;         // copy of disk inode
  uint16_t major;
  uint16_t minor;
  uint16_t nlink;
  uint32_t size;
  uint32_t addrs[NDIRECT+1];
};


struct pipe {
  struct spinlock lock;
  char data[128];
  uint32_t nread;     // number of bytes read
  uint32_t nwrite;    // number of bytes written
  int readopen;   // read fd is still open
  int writeopen;  // write fd is still open
};

typedef int fd_t;

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

#endif

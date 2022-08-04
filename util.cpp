#include "util.h"

// for mmap:
#include <iostream>

#include <cstring>

void handle_error(const char* msg) {
    perror(msg);
    exit(255);
}

void beToLe(void* buf, const char* ptr, size_t size)
{
  // big endian to little endian bytes
  for (size_t i=size-1; i > -1; --i ) {
      memcpy(&buf, ptr+i, 1);
  }
}

void leToBe(char* buf, const char* ptr, size_t size)
{
  // big endian to little endian bytes
  for (size_t i=0; i < size; ++i ) {
      memcpy(&buf, ptr+i, 1);
  }
}
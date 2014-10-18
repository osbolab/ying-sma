/*
 *Copyright (C) 2010-2011, The AROS Development Team. All rights reserved.
 */

#pragma once

#include <cstdint>

#if defined(_M_IX86) || defined(_M_X64)
#include <intrin.h>
#endif

#define set_bit(nr, addr)   (addr |=  (1 << nr)) 
#define clear_bit(nr, addr) (addr &= ~(1 << nr))
#define set_bit_a(nr, addr)   (addr[nr >> 5] |=   1 << (nr & 0x1f))
#define clear_bit_a(nr, addr) (addr[nr >> 5] &= ~(1 << (nr & 0x1f)))

#if !defined(__i386__) && !defined(__x86_64__) && !defined(_M_IX86) && !defined(_M_X64) 
static const int table[] = {
  -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
  4, 4,
  4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  5,
  5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6,
  6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6,
  6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7,
  7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7,
  7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7,
  7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
  7,
  7, 7, 7, 7, 7, 7, 7
};
#endif

#if defined(__i386__) || defined(__x86_64__)
  #define ls_bit(i) (__builtin_ffs(i) - 1)
#elif defined(_M_IX86) || defined(_M_X64)
  #define ls_bit(i) (tzcnt_(i))
int __inline tzcnt_(std::uint32_t value)
{
  unsigned long tz = 0;
  return (_BitScanForward(&tz, value) ? tz : -1);
}
#else
  #define ls_bit(i) ls_bit_(i)
  static inline int ls_bit_(int i)
  {

    unsigned int a;
    unsigned int x = i & -i;

    a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
    return table[x >> a] + a;
  }
#endif

#if defined(__i386__) || defined(__x86_64__)
  #define ms_bit(i) (i ? 31 - __builtin_clz(i) : -1)
#elif defined(_M_IX86) || defined(_M_X64)
  #define ms_bit(i) (i ? (31 -__lzcnt(i)) : -1)
#else
  #define ms_bit(i) ms_bit_(i)
  static inline int ms_bit_(int i)
  {
    unsigned int a;
    unsigned int x = (unsigned int)i;

    a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
    return table[x >> a] + a;
  }
#endif
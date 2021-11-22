#ifndef MISC_H
#define MISC_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "pico/time.h"

unsigned long millis();
unsigned long max(unsigned long a, unsigned long b);
uint32_t crc32(const void *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
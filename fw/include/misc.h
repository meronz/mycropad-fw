#ifndef MISC_H
#define MISC_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "pico/time.h"

unsigned long millis();
unsigned long max(unsigned long a, unsigned long b);

#ifdef __cplusplus
}
#endif

#endif
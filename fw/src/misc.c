#include "pico/time.h"

unsigned long millis()
{
    return (unsigned long)to_ms_since_boot(get_absolute_time());
}

unsigned long max(unsigned long a, unsigned long b)
{
    return a > b ? a : b;
}
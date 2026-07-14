#include <stdint.h>

extern volatile uint64_t timer_ticks;

void sleep(uint32_t milliseconds);
void init_timer(uint32_t frequency);
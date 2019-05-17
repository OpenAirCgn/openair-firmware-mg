#ifndef _PWMFAN_
#define _PWMFAN_

#include "mgos.h"

void fan_init();

void fan_tick();

/** Get the current fan RPM. defunct for now
@return RPM */
uint16_t fan_get_rpm(); 

/** set fan power
@param power fan power 0 (off) to 255 (full speed) */
void fan_set_power(uint8_t power);




#endif


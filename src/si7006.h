#ifndef _SI7006_H_
#define _SI7006_H_

#include "mgos.h"
#include "mgos_i2c.h"

typedef void (*si7xxx_cb)(float celsius, float rh, int temp_raw, int rh_raw);
bool si7006_read();
void si7006_init(si7xxx_cb si_cb);
bool si7006_tick();

#endif

// vim: et:sw=2:ts=2

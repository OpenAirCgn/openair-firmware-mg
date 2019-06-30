#pragma once

#include "mgos.h"

typedef void (*si7xxx_cb)(float celsius, float rh, int temp_raw, int rh_raw);
void si7006_start();
void si7006_stop();
bool si7006_init(si7xxx_cb si_cb);


// vim: et:sw=2:ts=2

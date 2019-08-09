#ifndef _NOISEMETER_H_
#define _NOISEMETER_H_

#include "stdint.h"
#include "mgos.h"
#include "mgos_i2c.h"

typedef void (*noisemeter_callback)(float dba, float dbc);

bool noisemeter_init(noisemeter_callback n_cb);
void noisemeter_start();
void noisemeter_stop();

#endif


#ifndef _NOISEMETER_H_
#define _NOISEMETER_H_

#include "stdint.h"
#include "mgos.h"
#include "mgos_i2c.h"

typedef void (*noisemeter_callback)(float dba, float dbc, float dba_1s, 
  float dba_5s, float dba_10s, float dba_30s, float dba_1m, float dba_3m,
  float dba_5m);

bool noisemeter_init(noisemeter_callback n_cb);
void noisemeter_start();
void noisemeter_stop();

#endif


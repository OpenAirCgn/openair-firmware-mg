#ifndef _SDS011_H_
#define _SDS011_H_

#include "mgos.h"


typedef void (*sds011_cb)(uint32_t pm25_ng, uint32_t pm10_ng);

bool sds011_init(sds011_cb cb);

void sds011_tick();


#endif
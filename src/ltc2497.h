#ifndef _LTC2497_H_
#define _LTC2497_H_

#include "mgos.h"
#include "mgos_i2c.h"

bool ltc2497_read(struct mgos_i2c *i2c, uint8_t addr, uint8_t *outChan, int *outVal);

#endif

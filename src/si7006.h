#ifndef _SI7006_H_
#define _SI7006_H_

#include "mgos.h"
#include "mgos_i2c.h"

bool si7006_read(struct mgos_i2c *i2c);

#endif
#include "si7006.h"
#include <stdio.h>

#define SI7006_I2C_ADDR 0x40
#define SI7006_CMD_MEAS_RH_NOHOLD 0xF5
#define SI7006_CMD_READ_TEMP_FROM_LAST_RH 0xE0
#define SI7006_RETRIES 100


bool si7006_read(struct mgos_i2c *i2c) {
  uint8_t cmd = SI7006_CMD_MEAS_RH_NOHOLD;
  uint8_t result[2];
  int retries = 0;
  int temp = -1;

  bool ok = mgos_i2c_write(i2c, SI7006_I2C_ADDR, &cmd, 1, false);
  if (ok) {
    ok = false;
    for (retries=0; retries<SI7006_RETRIES; retries++) {
      ok = mgos_i2c_read(i2c, SI7006_I2C_ADDR, &(result[0]), 2, false);
      if (ok) {
        break;
      }
    }
  }
  mgos_i2c_stop(i2c);
  
  if (ok) {
    temp = mgos_i2c_read_reg_w(i2c, SI7006_I2C_ADDR, SI7006_CMD_READ_TEMP_FROM_LAST_RH);
    if (temp < 0) {
      ok = false;
    }
  }
  if (ok) {
    int rhval = result[0]*256+result[1];
    float rh = ((rhval * 125.0f) / 65536.0f) - 6.0f;
    float celsius = ((temp * 175.72f) / 65536.0f) - 46.85f;
    printf("SI7006 read succeeded rh %f temp %f\n", rh, celsius);

  } else {
    printf("SI7006 read failed\n");
  }
  return ok;
}

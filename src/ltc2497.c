#include "ltc2497.h"
#include <stdio.h>

static int lastChannel = -1;

bool ltc2497_read(struct mgos_i2c *i2c, uint8_t addr, uint8_t *outChan, int *outVal) {

  uint8_t newChannel = (lastChannel+1) & 7;
  uint8_t cmd = 0xa0 | newChannel;
  bool ok = false;
  bool haveData = false;
  int val = 0;

  if (lastChannel < 0) {
    ok = mgos_i2c_write(i2c, addr, &cmd, 1, true);
  } else {
    uint8_t buf[3];
    ok = mgos_i2c_read_reg_n(i2c, addr, cmd, 3, &(buf[0]));
    if (ok) {
      haveData = true;
      val = (buf[0] << 16) | (buf[1] << 8) | buf[2];
      bool sig = val & 0x800000;
      bool msb = val & 0x400000;
      val = (val >> 6) & 0xffff;
      if (sig) {
        if (msb) {
          val = 0x10000; //positive overflow
        }
      } else {
        if (!msb) {
          val = -(0x10000); //regular negative value
        } else {
          val -= 0x10000; //negative overflow
        }
      }
    }
  }
  if (ok) {
    if (haveData) {
      if (outChan) *outChan = lastChannel;
      if (outVal) *outVal = val;
    }
    lastChannel = newChannel;
  }
  return ok && haveData;
}

#include "si7006.h"
#include "mgos.h"

#define SI7006_I2C_ADDR 0x40
#define SI7006_CMD_MEAS_RH_NOHOLD 0xF5
#define SI7006_CMD_READ_TEMP_FROM_LAST_RH 0xE0
#define SI7006_RETRIES 100

// experimental mongoose-os-libs/si7021-i2c
#include "mgos_si7021.h"
#include "broker.h"
struct mgos_si7021 * si_struct = NULL;
// TODO REMOVE

static struct mgos_i2c *i2c = NULL;
static si7xxx_cb si7x_cb;

void si7006_init(si7xxx_cb si_cb) {
  i2c = mgos_i2c_get_global();
  si7x_cb = si_cb;

  // > now do experimental library init.
  si_struct = mgos_si7021_create(i2c, SI7006_I2C_ADDR);
  // < end TODO REMOVE!

}
bool si7006_tick() {
  bool ok = false;
	ok = si7006_read();
  // > now do experimental library read.
  if (si_struct != NULL) {
    float t = mgos_si7021_getTemperature(si_struct);
    float h = mgos_si7021_getHumidity(si_struct);
    oa_broker_push(oa_si7006_temp_test, t*1000);
    oa_broker_push(oa_si7006_rh_test, h*100);
  }
  // < end TODO REMOVE!

  return ok;
}

bool si7006_read(){
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
    LOG(LL_DEBUG, ("SI7006 read succeeded rh %f temp %f\n", rh, celsius));
    si7x_cb(celsius, rh, temp, rhval);
  } else {
    LOG(LL_ERROR, ("SI7006 read failed\n"));
  }
  return true;
}

// vim: et:sw=2:ts=2

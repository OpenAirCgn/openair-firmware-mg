#include "si7006.h"
#include "mgos.h"

// mongoose-os-libs/si7021-i2c
#include "mgos_si7021.h"
#include "broker.h"

#define SI7006_I2C_ADDR 0x40

struct mgos_si7021 * si_struct = NULL;
static struct mgos_i2c *i2c = NULL;

static si7xxx_cb si7x_cb;

static mgos_timer_id timer_id = (mgos_timer_id)NULL;

bool si7006_init(si7xxx_cb si_cb) {
  si7x_cb = si_cb;
  si_struct = mgos_si7021_create(i2c, SI7006_I2C_ADDR);
  return true;

}

void si7006_tick(void * args) {
  if (si_struct != NULL) {
    float t = mgos_si7021_getTemperature(si_struct);
    float h = mgos_si7021_getHumidity(si_struct);
    oa_broker_push(oa_si7006_temp, t*1000);
    oa_broker_push(oa_si7006_rh, h*100);
  }
}

void si7006_start(){
  int interval = mgos_sys_config_get_openair_si7006_interval();
  timer_id = mgos_set_timer(interval, MGOS_TIMER_REPEAT, si7006_tick, NULL);
}

void si7006_stop(){
  if (timer_id) {
    mgos_clear_timer(timer_id);
    timer_id = (mgos_timer_id)NULL;
  }
}

// vim: et:sw=2:ts=2

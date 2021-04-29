#include "noisemeter.h"


static bool noisemeter_initialized = false;

static struct mgos_i2c *i2c;
static uint16_t addr = 0x10;  //unshifted (alt: 0x20)
static noisemeter_callback cb;
static mgos_timer_id timer_id = (mgos_timer_id)NULL;

bool noisemeter_init( noisemeter_callback n_cb ) {
  cb = n_cb;
  i2c = mgos_i2c_get_global();
  noisemeter_initialized = true;
  return true;
}

void noisemeter_tick() {
  float data[9] = { 0,0,0,0,0,0,0,0,0 };
  bool ok = mgos_i2c_read(i2c, addr, (uint8_t*)data, 9*sizeof(float), true);
  if (ok && cb) {
    cb(data[0],data[1],data[2],
       data[3],data[4],data[5],
       data[6],data[7],data[8]);
  }
}

void noisemeter_start(){
  int interval = mgos_sys_config_get_openair_noisemeter_interval();
  timer_id = mgos_set_timer(interval, MGOS_TIMER_REPEAT, noisemeter_tick, NULL);
}

void noisemeter_stop(){
  if (timer_id) {
    mgos_clear_timer(timer_id);
    timer_id = (mgos_timer_id)NULL;
  }
}

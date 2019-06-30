#include "quadsense.h"
#include "ltc2497.h"
#include "bme280.h"
#include "openairboard.h"

#include "stdio.h"

static bool alpha_initialized = false;
static bool bme0_initialized = false;
static bool bme1_initialized = false;

static BME280_Struct bme0;
static BME280_Struct bme1;

static struct mgos_i2c *i2c;
static uint8_t adc_addr = 0;
static bool id1 = false;

static int adc_values[8] = {0,0,0,0,0,0,0,0};
static alphasense_cb alpha_cb;
static bme280_cb bme_cb;

static mgos_timer_id timer_id_alpha = NULL;
static mgos_timer_id timer_id_bme0 = NULL;
static mgos_timer_id timer_id_bme1 = NULL;

//void quadsense_init() {
bool quadsense_init( alphasense_cb a_cb, bme280_cb b_cb ) {

  alpha_cb = a_cb;
  bme_cb = b_cb;

  LOG(LL_INFO, ("alpha_cb %p", alpha_cb));
  LOG(LL_INFO, ("bme_cb %p", bme_cb));

  i2c = mgos_i2c_get_global();
  int modIdx = mgos_sys_config_get_openair_quadsense_idx();

  switch (modIdx) {
    case 1: adc_addr = 0x16; break;
    case 2: adc_addr = 0x14; id1 = true; break;
    default:
            LOG(LL_ERROR, ("Quadsense init: Invalid module index %i", modIdx));
            break;
  }
  openair_enable_module(modIdx, true);
  alpha_initialized = true;
  LOG(LL_DEBUG, ("quadsense + alpha initialized"));


  if (mgos_sys_config_get_openair_quadsense_bme0_en()) {
    bme0_initialized = bme280_init(&bme0, i2c, 0);
    LOG(LL_INFO, ("quadsense + bme0 initialized"));
  }

  if (mgos_sys_config_get_openair_quadsense_bme1_en()) {
    bme1_initialized = bme280_init(&bme1, i2c, 1);
    LOG(LL_INFO, ("quadsense + bme1 initialized"));
  }

  return true;
}

#define VREF 4.11

static void alpha_tick(){
  if (alpha_initialized) {
    uint8_t chan;
    int val;

    if (ltc2497_read(i2c, adc_addr, &chan, &val)) {
      adc_values[chan] = val;
      if (chan==7) {
        for (int i=0; i<8; i++) {
          LOG(LL_DEBUG, ("Channel %i value %i - %fV", i, adc_values[i], 0.5*VREF*adc_values[i]/65536.0));
        }
        alpha_cb(
            adc_values[0],
            adc_values[1],
            adc_values[2],
            adc_values[3],
            adc_values[4],
            adc_values[5],
            adc_values[6],
            adc_values[7]
            );
      }
    } else {
      LOG(LL_ERROR, ("ltc2497 read failed"));
    }
  }
}
static void bme_tick(int bmeNo){
  uint32_t temp, press, hum;
  BME280_Struct * bme = bmeNo == 0 ? &bme0 : &bme1;

  bool ok = bme280_read_data(bme, &temp, &press, &hum); //TODO: ID must be set for new HW
  if (ok && bme_cb) {
    float realTemp, realPress, realHum;
    bme280_compensate(bme, temp, press, hum, &realTemp, &realPress, &realHum);
    bme_cb(bmeNo, press, realPress, temp, realTemp, hum, realHum);
  }
  if (!ok) {
    bme_cb(bmeNo,0,0,0,0,0,0);
    LOG(LL_ERROR, ("bme280(%d) read failed: %d",bmeNo, ok));
  }
}

static void bme0_tick(){
  if (bme0_initialized) { 
    bme_tick(0);
  } else {
    bme0_initialized = bme280_init(&bme0, i2c, 0);
  }
}
static void bme1_tick(){
  if (bme1_initialized) { 
    bme_tick(1);
  } else {
    bme1_initialized = bme280_init(&bme1, i2c, 1);
  }
}

void quadsense_start(){
  int interval;
  if (mgos_sys_config_get_openair_quadsense_en()){
    interval = mgos_sys_config_get_openair_quadsense_interval();
    timer_id_alpha = mgos_set_timer(interval, MGOS_TIMER_REPEAT, alpha_tick, NULL);
  }
  if (mgos_sys_config_get_openair_quadsense_bme0_en()){
    interval = mgos_sys_config_get_openair_quadsense_bme0_interval();
    timer_id_alpha = mgos_set_timer(interval, MGOS_TIMER_REPEAT, bme0_tick, NULL);
  }
  if (mgos_sys_config_get_openair_quadsense_bme1_en()){
    interval = mgos_sys_config_get_openair_quadsense_bme1_interval();
    timer_id_alpha = mgos_set_timer(interval, MGOS_TIMER_REPEAT, bme1_tick, NULL);
  }
}

void quadsense_stop(){
 if (timer_id_alpha) {
  mgos_clear_timer(timer_id_alpha);
  timer_id_alpha = (mgos_timer_id)NULL;
 }
 if (timer_id_bme0) {
  mgos_clear_timer(timer_id_bme0);
  timer_id_bme0 = (mgos_timer_id)NULL;
 }
 if (timer_id_bme1) {
  mgos_clear_timer(timer_id_bme1);
  timer_id_bme1 = (mgos_timer_id)NULL;
 }
}


// vim: et:sw=2:ts=2

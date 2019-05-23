#include "quadsense.h"
#include "ltc2497.h"
#include "bme280.h"
#include "openairboard.h"

#include "stdio.h"

static bool initialized = false;
static struct mgos_i2c *i2c;
static uint8_t adc_addr = 0;
static bool id1 = false;
static int adc_values[8] = {0,0,0,0,0,0,0,0};

static alphasense_cb alpha_cb;
static bme280_cb bme_cb;

//void quadsense_init() {
void quadsense_init( alphasense_cb a_cb, bme280_cb b_cb ) {

  alpha_cb = a_cb;
  bme_cb = b_cb;

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


  bool ok = bme280_read_calib(i2c, false);  //TODO: ID must be set for new HW
  if (!ok) {
    LOG(LL_ERROR, ("failed bme280_read_calib()"));
    return;
  }

  ok = bme280_set_mode(i2c, false,     //TODO: ID must be set for new HW
    MEASURE_OS4,
    MEASURE_OS4,
    MEASURE_OS4,
    MODE_NORMAL,
    STANDBY_62MS5,
    FILTER_2);

  if (!ok) {
    LOG(LL_ERROR, ("failed bme_set_mode"));
    return;
  }
  initialized = true;
  LOG(LL_INFO, ("quadsense initialized"));
}

#define VREF 4.11

bool quadsense_tick() {
  if (!initialized) {
    return false;
  }
  uint8_t chan;
  int val;
  bool ok = ltc2497_read(i2c, adc_addr, &chan, &val);

  if (ok) {
    adc_values[chan] = val;
    if (chan==7) {
      for (int i=0; i<8; i++) {
        LOG(LL_INFO, ("Channel %i value %i - %fV", i, adc_values[i], 0.5*VREF*adc_values[i]/65536.0));
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
    }
  } else {
    LOG(LL_ERROR, ("ltc2497 read failed: %d",ok));
  }

  uint32_t temp, press, hum;
  ok = bme280_read_data(i2c, false, &temp, &press, &hum); //TODO: ID must be set for new HW
  if (ok && bme_cb) {
    float realTemp, realPress, realHum;
    bme280_compensate(temp, press, hum, &realTemp, &realPress, &realHum);
    bme_cb(press, realPress, temp, realTemp, hum, realHum);
  }
  if (!ok) {
    LOG(LL_ERROR, ("bme280 read failed: %d",ok));
  }
  return ok;
}

// vim: et:sw=2:ts=2

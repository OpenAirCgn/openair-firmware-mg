#include "quadsense.h"
#include "alphasense.h"
#include "bme280.h"
#include "ltc2497.h"
#include "openairboard.h"

#include "stdio.h"

static bool alpha_initialized = false;
static bool bme0_initialized = false;
static bool bme1_initialized = false;

static BME280_Struct bme0;
static BME280_Struct bme1;

static struct mgos_i2c *i2c;

// Quadsense design changed and this needs to to be rethought ... There
// used to be a configurable i2c addr pin, but this became irrelevant b/c
// Quadsense got 2 BME280 Temp sensors and those only have two configurable
// addr.  Simultaneously, the addr configuration pins of the ADC got
// inverted, so the old addr mapping of MOD1 => 0x16 MOD2 =>0x14 got
// swapped. The constants below refer to the NEW mapping.
//
// As a consequence: we can't have two Quadsense boards (at least not with
// 2 BME sensors each.) And we'll autodiscover the ADC addr.
//
// To make matters worse, the modular ports were modified on a later
// hardware revision (1.5) to facilitate more dynamic line on the ports.
// Module PINS 4 & 6 (ID1 & ID2) were mapped to SENSx_IO2 and SENSx_IN
// respectively: These signals are connected to: ESP: IO_14 and IO_17 as
// well as IO_34 and IO_35. These ESP PINs were previously connected to
// the on board MICs NOx sensor, which is no longer on the board.

#define ADC_MOD1_ADDR 0x14
#define ADC_MOD2_ADDR 0x16

static uint8_t adc_addr = 0;

static int adc_values[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static alphasense_cb alpha_cb;
static bme280_cb bme_cb;

static mgos_timer_id timer_id_alpha = (mgos_timer_id)NULL;
static mgos_timer_id timer_id_bme0 = (mgos_timer_id)NULL;
static mgos_timer_id timer_id_bme1 = (mgos_timer_id)NULL;

// void quadsense_init() {
bool quadsense_init(alphasense_cb a_cb, bme280_cb b_cb) {

  alpha_cb = a_cb;
  bme_cb = b_cb;

  LOG(LL_INFO, ("alpha_cb %p", alpha_cb));
  LOG(LL_INFO, ("bme_cb %p", bme_cb));

  i2c = mgos_i2c_get_global();
  int modIdx = mgos_sys_config_get_openair_quadsense_idx();

  switch (modIdx) {
  case 1:
    adc_addr = ADC_MOD1_ADDR;
    break;
  case 2:
    adc_addr = ADC_MOD2_ADDR;
    break;
  default:
    LOG(LL_ERROR, ("Quadsense init: Invalid module index %i", modIdx));
    break;
  }
  openair_enable_module(modIdx, true);
  alpha_initialized = true;
  LOG(LL_INFO, ("quadsense + alpha initialized"));

  if (mgos_sys_config_get_openair_quadsense_bme0_en()) {
    bme0_initialized = bme280_init(&bme0, i2c, 0);
    if (bme0_initialized) {
      LOG(LL_INFO, ("quadsense/bme0 initialized"));
    } else {
      LOG(LL_INFO, ("quadsense/bme0 initialization failed"));
    }
  }

  if (mgos_sys_config_get_openair_quadsense_bme1_en()) {
    bme1_initialized = bme280_init(&bme1, i2c, 1);
    if (bme1_initialized) {
      LOG(LL_INFO, ("quadsense/bme1 initialized"));
    } else {
      LOG(LL_INFO, ("quadsense/bme1 initialization failed"));
    }
  }

  return true;
}

#define VREF 4.11

/* returns ppb value for a given sensor.
Calculations according to Alphasense AAN 803-05 */
static int alpha_calc(int idx) {
  int weAdc, aeAdc, algoIdx, we0, ae0, weSens, gain, sensitivity, wee, aee;
  AlphaSensorType type;
  switch (idx) {
  case 1:
    weAdc = adc_values[0];
    aeAdc = adc_values[1];
    we0 = mgos_sys_config_get_quadsense_alpha1_wezero();
    ae0 = mgos_sys_config_get_quadsense_alpha1_aezero();
    weSens = mgos_sys_config_get_quadsense_alpha1_wesens();
    gain = mgos_sys_config_get_quadsense_alpha1_gain();
    sensitivity = mgos_sys_config_get_quadsense_alpha1_sensitivity();
    wee = mgos_sys_config_get_quadsense_alpha1_ewezero();
    aee = mgos_sys_config_get_quadsense_alpha1_eaezero();
    algoIdx = mgos_sys_config_get_quadsense_alpha1_algo();
    type = mgos_sys_config_get_quadsense_alpha1_type();
    break;
  case 2:
    weAdc = adc_values[2];
    aeAdc = adc_values[3];
    we0 = mgos_sys_config_get_quadsense_alpha2_wezero();
    ae0 = mgos_sys_config_get_quadsense_alpha2_aezero();
    weSens = mgos_sys_config_get_quadsense_alpha2_wesens();
    gain = mgos_sys_config_get_quadsense_alpha2_gain();
    sensitivity = mgos_sys_config_get_quadsense_alpha2_sensitivity();
    wee = mgos_sys_config_get_quadsense_alpha2_ewezero();
    aee = mgos_sys_config_get_quadsense_alpha2_eaezero();
    algoIdx = mgos_sys_config_get_quadsense_alpha2_algo();
    type = mgos_sys_config_get_quadsense_alpha2_type();
    break;
  case 3:
    weAdc = adc_values[4];
    aeAdc = adc_values[5];
    we0 = mgos_sys_config_get_quadsense_alpha3_wezero();
    ae0 = mgos_sys_config_get_quadsense_alpha3_aezero();
    weSens = mgos_sys_config_get_quadsense_alpha3_wesens();
    gain = mgos_sys_config_get_quadsense_alpha3_gain();
    sensitivity = mgos_sys_config_get_quadsense_alpha3_sensitivity();
    wee = mgos_sys_config_get_quadsense_alpha3_ewezero();
    aee = mgos_sys_config_get_quadsense_alpha3_eaezero();
    algoIdx = mgos_sys_config_get_quadsense_alpha3_algo();
    type = mgos_sys_config_get_quadsense_alpha3_type();
    break;
  case 4:
    weAdc = adc_values[6];
    aeAdc = adc_values[7];
    we0 = mgos_sys_config_get_quadsense_alpha4_wezero();
    ae0 = mgos_sys_config_get_quadsense_alpha4_aezero();
    weSens = mgos_sys_config_get_quadsense_alpha4_wesens();
    gain = mgos_sys_config_get_quadsense_alpha4_gain();
    sensitivity = mgos_sys_config_get_quadsense_alpha4_sensitivity();
    wee = mgos_sys_config_get_quadsense_alpha4_ewezero();
    aee = mgos_sys_config_get_quadsense_alpha4_eaezero();
    algoIdx = mgos_sys_config_get_quadsense_alpha4_algo();
    type = mgos_sys_config_get_quadsense_alpha4_type();
    break;
  default:
    LOG(LL_ERROR, ("alpha_calc: Invalid index %i (must be 1-4)", idx));
    return -1;
  }
  // obtain voltages in volts
  float weu = 0.5 * VREF * adc_values[2 * (idx - 1)] / 65536.0;     // Volts
  float aeu = 0.5 * VREF * adc_values[2 * (idx - 1) + 1] / 65536.0; // Volts

  // compensate electronic zero offsets -- TODO same as wee, aee?
  // float we = weu - (wee * 0.001);
  // float ae = aeu - (aee * 0.001);

  float celcius = 20.0;

  float ppm = alphaCalculatePPM(type, celcius, weu, aeu, wee, aee, we0, ae0,
                                sensitivity);
  return ppm;
}

static void alpha_tick() {
  if (alpha_initialized) {
    uint8_t chan;
    int val;

    if (ltc2497_read(i2c, adc_addr, &chan, &val)) {
      adc_values[chan] = val;
      if (chan == 7) {
        for (int i = 0; i < 8; i++) {
          LOG(LL_DEBUG, ("Channel %i value %i - %fV", i, adc_values[i],
                         0.5 * VREF * adc_values[i] / 65536.0));
        }
        uint32_t ppb1 = alpha_calc(1);
        uint32_t ppb2 = alpha_calc(2);
        uint32_t ppb3 = alpha_calc(3);
        uint32_t ppb4 = alpha_calc(4);
        alpha_cb(adc_values[0], adc_values[1], adc_values[2], adc_values[3],
                 adc_values[4], adc_values[5], adc_values[6], adc_values[7],
                 ppb1, ppb2, ppb3, ppb4);
      }
    } else {
      LOG(LL_ERROR, ("ltc2497 read failed. failed addr: %d", adc_addr));
      adc_addr = adc_addr == ADC_MOD1_ADDR ? ADC_MOD2_ADDR : ADC_MOD1_ADDR;
    }
  }
}
static void bme_tick(int bmeNo) {
  uint32_t temp, press, hum;
  BME280_Struct *bme = bmeNo == 0 ? &bme0 : &bme1;

  bool ok = bme280_read_data(bme, &temp, &press,
                             &hum); // TODO: ID must be set for new HW
  if (ok && bme_cb) {
    float realTemp, realPress, realHum;
    bme280_compensate(bme, temp, press, hum, &realTemp, &realPress, &realHum);
    bme_cb(bmeNo, press, realPress, temp, realTemp, hum, realHum);
  }
  if (!ok) {
    bme_cb(bmeNo, 0, 0, 0, 0, 0, 0);
    LOG(LL_ERROR, ("bme280(%d) read failed: %d", bmeNo, ok));
  }
}

static int bme0_attempts = 3;
static void bme0_tick() {
  if (bme0_initialized) {
    bme_tick(0);
  } else if (bme0_attempts != 0) {
    bme0_initialized = bme280_init(&bme0, i2c, 0);
    --bme0_attempts;
  }
}
static int bme1_attempts = 3;
static void bme1_tick() {
  if (bme1_initialized) {
    bme_tick(1);
  } else if (bme1_attempts != 0) {
    bme1_initialized = bme280_init(&bme1, i2c, 1);
    --bme1_attempts;
  }
}

void quadsense_start() {
  int interval;
  if (mgos_sys_config_get_openair_quadsense_en()) {
    interval = mgos_sys_config_get_openair_quadsense_interval();
    timer_id_alpha =
        mgos_set_timer(interval, MGOS_TIMER_REPEAT, alpha_tick, NULL);
  }
  if (mgos_sys_config_get_openair_quadsense_bme0_en()) {
    interval = mgos_sys_config_get_openair_quadsense_bme0_interval();
    timer_id_alpha =
        mgos_set_timer(interval, MGOS_TIMER_REPEAT, bme0_tick, NULL);
  }
  if (mgos_sys_config_get_openair_quadsense_bme1_en()) {
    interval = mgos_sys_config_get_openair_quadsense_bme1_interval();
    timer_id_alpha =
        mgos_set_timer(interval, MGOS_TIMER_REPEAT, bme1_tick, NULL);
  }
}

void quadsense_stop() {
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

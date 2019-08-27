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

/* returns ppb value for a given sensor.
Calculations according to Alphasense AAN 803-05 */
static int alpha_calc(int idx) {
  int weAdc, aeAdc, algoIdx, weZero, aeZero, weSens, gain1, gain2, weEZero, aeEZero;
  switch (idx) {
    case 1:
      weAdc = adc_values[0];
      aeAdc = adc_values[1];
      weZero = mgos_sys_config_get_quadsense_alpha1_wezero();
      aeZero = mgos_sys_config_get_quadsense_alpha1_aezero();
      weSens = mgos_sys_config_get_quadsense_alpha1_wesens();
      gain1 = mgos_sys_config_get_quadsense_alpha1_gain1();
      gain2 = mgos_sys_config_get_quadsense_alpha1_gain2();
      weEZero = mgos_sys_config_get_quadsense_alpha1_ewezero();
      aeEZero = mgos_sys_config_get_quadsense_alpha1_eaezero();
      algoIdx = mgos_sys_config_get_quadsense_alpha1_algo();
      break;
    case 2:
      weAdc = adc_values[2];
      aeAdc = adc_values[3];
      weZero = mgos_sys_config_get_quadsense_alpha2_wezero();
      aeZero = mgos_sys_config_get_quadsense_alpha2_aezero();
      weSens = mgos_sys_config_get_quadsense_alpha2_wesens();
      gain1 = mgos_sys_config_get_quadsense_alpha2_gain1();
      gain2 = mgos_sys_config_get_quadsense_alpha2_gain2();
      weEZero = mgos_sys_config_get_quadsense_alpha2_ewezero();
      aeEZero = mgos_sys_config_get_quadsense_alpha2_eaezero();
      algoIdx = mgos_sys_config_get_quadsense_alpha2_algo();
      break;
    case 3:
      weAdc = adc_values[4];
      aeAdc = adc_values[5];
      weZero = mgos_sys_config_get_quadsense_alpha3_wezero();
      aeZero = mgos_sys_config_get_quadsense_alpha3_aezero();
      weSens = mgos_sys_config_get_quadsense_alpha3_wesens();
      gain1 = mgos_sys_config_get_quadsense_alpha3_gain1();
      gain2 = mgos_sys_config_get_quadsense_alpha3_gain2();
      weEZero = mgos_sys_config_get_quadsense_alpha3_ewezero();
      aeEZero = mgos_sys_config_get_quadsense_alpha3_eaezero();
      algoIdx = mgos_sys_config_get_quadsense_alpha3_algo();
      break;
    case 4:
      weAdc = adc_values[6];
      aeAdc = adc_values[7];
      weZero = mgos_sys_config_get_quadsense_alpha4_wezero();
      aeZero = mgos_sys_config_get_quadsense_alpha4_aezero();
      weSens = mgos_sys_config_get_quadsense_alpha4_wesens();
      gain1 = mgos_sys_config_get_quadsense_alpha4_gain1();
      gain2 = mgos_sys_config_get_quadsense_alpha4_gain2();
      weEZero = mgos_sys_config_get_quadsense_alpha4_ewezero();
      aeEZero = mgos_sys_config_get_quadsense_alpha4_eaezero();
      algoIdx = mgos_sys_config_get_quadsense_alpha4_algo();
      break;
    default:
      LOG(LL_ERROR, ("alpha_calc: Invalid index %i (must be 1-4)", idx));
      return -1;
  }
  //obtain voltages in volts
  float weu = 0.5*VREF*adc_values[2*(idx-1)]/65536.0;    //Volts
  float aeu = 0.5*VREF*adc_values[2*(idx-1)+1]/65536.0;  //Volts
  //compensate electronic zero offsets
  float we = weu - (weEZero * 0.001);
  float ae = aeu - (aeEZero * 0.001);


  return 123; //TODO **************
}

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
        uint32_t ppb1 = alpha_calc(1);
        uint32_t ppb2 = alpha_calc(2);
        uint32_t ppb3 = alpha_calc(3);
        uint32_t ppb4 = alpha_calc(4);
        alpha_cb(
            adc_values[0],
            adc_values[1],
            adc_values[2],
            adc_values[3],
            adc_values[4],
            adc_values[5],
            adc_values[6],
            adc_values[7],
            ppb1, ppb2, ppb3, ppb4
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

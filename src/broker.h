#pragma once

#include "mgos.h"
#include "mgos_rpc.h"


typedef enum {
  oa_time, // last time recv
  oa_alpha_1,
  oa_alpha_2,
  oa_alpha_3,
  oa_alpha_4,
  oa_alpha_5,
  oa_alpha_6,
  oa_alpha_7,
  oa_alpha_8,
  oa_bme0_pressure_raw,
  oa_bme0_pressure,  //pascal
  oa_bme0_temp_raw,
  oa_bme0_temp,      //millikelvin
  oa_bme0_humidity_raw,
  oa_bme0_humidity,  //permille rh
  oa_bme1_pressure_raw,
  oa_bme1_pressure,  //pascal
  oa_bme1_temp_raw,
  oa_bme1_temp,      //millikelvin
  oa_bme1_humidity_raw,
  oa_bme1_humidity,  //permille rh
  oa_sds_pm25,       //PM2.5 in ng/m3
  oa_sds_pm10,       //PM10 in ng/m3
  oa_si7006_temp_raw,
  oa_si7006_temp,
  oa_si7006_rh_raw,
  oa_si7006_rh,
  oa_mics4514_vred,
  oa_mics4514_vox,
  oa_dba,            //millibel = decibel * 100
  oa_dbc,            //millibel = decibel * 100
  oa_alpha_1_ppb,
  oa_alpha_2_ppb,
  oa_alpha_3_ppb,
  oa_alpha_4_ppb,
  oa_dba_1s,         //millibel = decibel * 100
  oa_dba_5s,         //millibel = decibel * 100
  oa_dba_10s,        //millibel = decibel * 100
  oa_dba_30s,        //millibel = decibel * 100
  oa_dba_1m,         //millibel = decibel * 100
  oa_dba_3m,         //millibel = decibel * 100
  oa_dba_5m          //millibel = decibel * 100

} oa_tag;

#define NUM_VALUES 42 /* 40? */


typedef struct {
  uint32_t ts;
  uint32_t value;
} oa_entry;

bool oa_broker_init();
void oa_broker_tick();
void oa_broker_push(oa_tag tag, uint32_t value);

// Callbacks
void mics_cb(int vred, int vox);
void si7006_cb(float celsius, float rh, int temp, int rh_raw);

void sds_cb(uint32_t pm25, uint32_t pm10); 
void alpha_cb(
    int alpha1,
    int alpha2,
    int alpha3,
    int alpha4,
    int alpha5,
    int alpha6,
    int alpha7,
    int alpha8,
    int ppb1,
    int ppb2,
    int ppb3,
    int ppb4
    ) ;
void bme_cb(uint8_t idx, 
    uint32_t p_raw, float p, 
    uint32_t t_raw, float t, 
    uint32_t h_raw, float h);

void noisemeter_cb(float dba, float dbc, float dba_1s, 
  float dba_5s, float dba_10s, float dba_30s, float dba_1m, float dba_3m,
  float dba_5m);

// vim: et:sw=2:ts=2

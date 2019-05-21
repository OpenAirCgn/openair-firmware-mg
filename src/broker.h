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
  oa_bme_pressure_raw,
  oa_bme_pressure,
  oa_bme_tmp_raw,
  oa_bme_tmp,
  oa_bme_humidity_raw,
  oa_bme_humidity
} oa_tag;

#define NUM_VALUES 15


typedef struct {
  uint32_t ts;
  uint32_t value;
} oa_entry;

bool oa_broker_init();
void oa_broker_tick();
void oa_broker_push(oa_tag tag, uint32_t value);



// vim: et:sw=2:ts=2

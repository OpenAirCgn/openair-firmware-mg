
#include <stdio.h>

#include "mgos.h"

#include "openairboard.h"
#include "quadsense.h"
#include "sds011.h"
#include "broker.h"
#include "sds011.h"

static uint32_t ticks = 0;

static void alpha_cb(
    int alpha1,
    int alpha2,
    int alpha3,
    int alpha4,
    int alpha5,
    int alpha6,
    int alpha7,
    int alpha8
    ) {
  oa_broker_push(oa_alpha_1, alpha1);
  oa_broker_push(oa_alpha_2, alpha2);
  oa_broker_push(oa_alpha_3, alpha3);
  oa_broker_push(oa_alpha_4, alpha4);
  oa_broker_push(oa_alpha_5, alpha5);
  oa_broker_push(oa_alpha_6, alpha6);
  oa_broker_push(oa_alpha_7, alpha7);
  oa_broker_push(oa_alpha_8, alpha8);
}


void bme_cb(
    uint32_t p_raw,
    float p,
    uint32_t t_raw,
    float t,
    uint32_t h_raw,
    float h
    ) {
    oa_broker_push(oa_bme_pressure_raw, p_raw);
    oa_broker_push(oa_bme_pressure, (uint32_t)(p * 100));
    oa_broker_push(oa_bme_tmp_raw, t_raw);
    oa_broker_push(oa_bme_tmp, (uint32_t)((t + 273.15) * 1000));
    oa_broker_push(oa_bme_humidity_raw, h_raw);
    oa_broker_push(oa_bme_humidity, (uint32_t)(h*10));
}

static void sds_cb(uint32_t pm25, uint32_t pm10) {
  oa_broker_push(oa_sds_pm25, pm25);
  oa_broker_push(oa_sds_pm10, pm10);
}

static void timer_cb(void *arg) {
  (void) arg;
  openair_tick();

  if (mgos_sys_config_get_openair_quadsense_en()) {
    quadsense_tick();
  }
  if (mgos_sys_config_get_openair_sds011_en()) {
    sds011_tick();
  }
}

enum mgos_app_init_result mgos_app_init(void) {
  LOG(LL_INFO, ("OpenAir starting..."));

  openair_init();
  oa_broker_init();

  if (mgos_sys_config_get_openair_quadsense_en()) {
    quadsense_init(&alpha_cb, &bme_cb);
  }

  if (mgos_sys_config_get_openair_sds011_en()) {
    sds011_init(&sds_cb);
  }

  mgos_set_timer(150 /* ms */, true /* repeat */, timer_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}

// vim: et:sw=2:ts=2

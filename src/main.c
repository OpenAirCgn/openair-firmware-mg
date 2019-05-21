
#include <stdio.h>

#include "mgos.h"

#include "openairboard.h"
#include "quadsense.h"
#include "broker.h"



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

static uint32_t ticks = 0;

static void timer_cb(void *arg) {
  (void) arg;
  openair_tick();
  if (mgos_sys_config_get_openair_quadsense_en()) {
    quadsense_tick();
  }

  oa_broker_push(oa_time, ticks++);

}

enum mgos_app_init_result mgos_app_init(void) {
  LOG(LL_INFO, ("OpenAir starting..."));

  openair_init();
  oa_broker_init();


  if (mgos_sys_config_get_openair_quadsense_en()) {
    quadsense_init(&alpha_cb, NULL);
  }

  mgos_set_timer(150 /* ms */, true /* repeat */, timer_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}

// vim: et:sw=2:ts=2

#include <stdio.h>

#include "mgos.h"
#include "mgos_wifi.h"

#include "mgos_mongoose.h"

#include "broker.h"
#include "mics4514.h"
#include "noisemeter.h"
#include "openairboard.h"
#include "quadsense.h"
#include "sds011.h"
#include "si7006.h"

static void check_connection() {
  enum mgos_wifi_status status = mgos_wifi_get_status();
  openair_setStatusPattern((status == MGOS_WIFI_IP_ACQUIRED)
                               ? OA_BLINK_ONCE_PAUSE
                               : OA_BLINK_TWICE_PAUSE);
}

static void timer_cb(void *arg) {
  (void)arg;
  check_connection();
  openair_tick();
}

enum mgos_app_init_result mgos_app_init(void) {
  LOG(LL_INFO, ("OpenAir starting..."));

  openair_init();
  oa_broker_init();

  if (mgos_sys_config_get_openair_noisemeter_en()) {
    if (noisemeter_init(&noisemeter_cb)) {
      noisemeter_start();
    }
  }

  if (mgos_sys_config_get_openair_quadsense_en()) {
    if (quadsense_init(&alpha_cb, &bme_cb)) {
      quadsense_start();
    }
  }

  if (mgos_sys_config_get_openair_sds011_en()) {
    if (sds011_init(&sds_cb)) {
      sds011_start();
    }
  }

  if (mgos_sys_config_get_openair_si7006_en()) {
    if (si7006_init(&si7006_cb)) {
      si7006_start();
    }
  }

  //  if (mgos_sys_config_get_openair_mics4514_en()) {
  //    if (mics4514_init(&mics_cb)) {
  //      mics4514_start();
  //    }
  //  }

  int tick_interval = mgos_sys_config_get_openair_tick_interval();
  mgos_set_timer(tick_interval /* ms */, true /* repeat */, timer_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}

// vim: et:sw=2:ts=2

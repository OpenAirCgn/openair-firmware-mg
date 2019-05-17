
#include <stdio.h>

#include "mgos.h"

#include "openairboard.h"
#include "quadsense.h"




static void timer_cb(void *arg) {
  (void) arg;
  openair_tick();
  if (mgos_sys_config_get_openair_quadsense_en()) {
    quadsense_tick();
  }
}

enum mgos_app_init_result mgos_app_init(void) {
  LOG(LL_INFO, ("OpenAir starting..."));

  openair_init();


  if (mgos_sys_config_get_openair_quadsense_en()) {
    quadsense_init();
  }

  mgos_set_timer(150 /* ms */, true /* repeat */, timer_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}

// vim: et:sw=2:ts=2

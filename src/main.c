
#include <stdio.h>

#include "mgos.h"
#include "quadsense.h"
#include "openairboard.h"

#define PM_UART 2



static void timer_cb(void *arg) {
  (void) arg;
  openair_tick();
  if (mgos_sys_config_get_openair_quadsense_en()) {
    quadsense_tick();
  }
}

static void handle_pm_data(int uart_no, void *arg) {
  static struct mbuf lb = {0};
  size_t rx_av = mgos_uart_read_avail(uart_no);
  if (rx_av > 0) {
    mgos_uart_read_mbuf(uart_no, &lb, rx_av);
    LOG(LL_INFO, ("Got data from PM\n"));
    mbuf_clear(&lb);
  }
}

enum mgos_app_init_result mgos_app_init(void) {
  LOG(LL_INFO, ("OpenAir starting..."));

  openair_init();
  //openair_enable_module(2, true); //assume Quadsense is connected to it. TODO: Config

  if (mgos_sys_config_get_openair_quadsense_en()) {
    quadsense_init();
  }


//  //init PM sensor UART
//  struct mgos_uart_config ucfg;
//  mgos_uart_config_set_defaults(PM_UART, &ucfg);
//  ucfg.baud_rate = 96000;
//  ucfg.num_data_bits = 8;
//  ucfg.parity = MGOS_UART_PARITY_NONE;
//  ucfg.stop_bits = MGOS_UART_STOP_BITS_1;
//  if (!mgos_uart_configure(PM_UART, &ucfg)) {
//    return MGOS_APP_INIT_ERROR;
//  }
//  mgos_uart_set_dispatcher(PM_UART, handle_pm_data, NULL /* arg */);
//  mgos_uart_set_rx_enabled(PM_UART, true);


  mgos_set_timer(150 /* ms */, true /* repeat */, timer_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}

// vim: et:sw=2:ts=2

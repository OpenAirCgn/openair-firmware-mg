#include "sds011.h"

static const int uart_no = 2;
static bool inited = false;
static sds011_cb cb = NULL;
static uint32_t pm25_accum = 0; //we sum up in ng/m3, reports are in 0.1ug/m3
static uint32_t pm10_accum = 0; //we sum up in ng/m3, reports are in 0.1ug/m3
static uint32_t num_measurements = 0;

static mgos_timer_id timer_id = (mgos_timer_id)0;

typedef enum {
  SDS011_SET_DATA_REPORTING_MODE = 2,
  SDS011_QUERY_DATA = 4,
  SDS011_SET_DEVICE_ID = 5,
  SDS011_SET_SLEEP_WORK = 6,
  SDS011_CHECK_FIRMWARE_VERSION = 7,
  SDS011_SET_WORK_PERIOD = 8
} SDS011_CMD_ID;

void sds011_uart_cb(int uart_no, void* arg);
void sds011_sendCommand(uint8_t cmdId, uint8_t data2, uint8_t data3, uint16_t devId);
uint8_t sds011_checksum(uint8_t* buf, uint8_t totalLen);

bool sds011_init(sds011_cb in_cb) {
  struct mgos_uart_config ucfg;
  mgos_uart_config_set_defaults(uart_no, &ucfg);

  ucfg.baud_rate = 9600;
  ucfg.rx_buf_size = 1500;
  ucfg.tx_buf_size = 1500;
  ucfg.num_data_bits = 8;
  ucfg.parity = MGOS_UART_PARITY_NONE;
  ucfg.stop_bits = MGOS_UART_STOP_BITS_1;
  ucfg.rx_fc_type = MGOS_UART_FC_NONE;
  ucfg.tx_fc_type = MGOS_UART_FC_NONE;
  ucfg.rx_linger_micros = 30;
  ucfg.dev.tx_gpio = 16;
  ucfg.dev.rx_gpio = 17;

  if (!mgos_uart_configure(uart_no, &ucfg)) {
    LOG(LL_ERROR, ("Failed to configure UART%d", uart_no));
    return false;
  }
  mgos_uart_set_dispatcher(uart_no, sds011_uart_cb, NULL);
  mgos_uart_set_rx_enabled(uart_no, true);
  //set to polling mode
  sds011_sendCommand(SDS011_SET_DATA_REPORTING_MODE, 1, 1, 0xffff);
  inited = true;
  cb = in_cb;
  return true;
}

#define POLL_INTERVAL 20
#define SLEEP_TICK 100
#define REPEAT_TICK 200

void sds011_tick() {
  if (!inited) {
    return;
  }
  static uint32_t counter = REPEAT_TICK;  //start at next tick
  if (counter == 0) {  //start of active period: activate, clear accumulators
    sds011_sendCommand(SDS011_SET_SLEEP_WORK, 1, 1, 0xffff);
    pm25_accum = 0;
    pm10_accum = 0;
    num_measurements = 0;
  }
  if ((counter > 0) && (counter < SLEEP_TICK) && ((counter % POLL_INTERVAL) == 0)) {  //poll data
     sds011_sendCommand(SDS011_QUERY_DATA, 0, 0, 0xffff);
  }
  if (counter == SLEEP_TICK) {  //end of active period: push results, turn off sensor
     sds011_sendCommand(SDS011_SET_SLEEP_WORK, 1, 0, 0xffff);
     if ((num_measurements > 0) && (cb)) {
        (*cb)(pm25_accum / num_measurements, pm10_accum / num_measurements);
     }
  }
  counter = (counter+1) % REPEAT_TICK;
}

void sds011_uart_cb(int uart_no, void* arg) {
  size_t avail = mgos_uart_read_avail(uart_no);
  uint8_t buf[100];
  if (avail > 100) {
    avail = 100;
  }
  if (avail > 0) {
    size_t read = mgos_uart_read(uart_no, buf, avail);
    if (read == 10) {
      if ((buf[0] == 0xaa) && (buf[9] == 0xab) && (buf[8] == sds011_checksum(buf, 10))) {
        if (buf[1] == 0xc0) { //measurement
          int pm25 = buf[2] | (buf[3] << 8);
          int pm10 = buf[4] | (buf[5] << 8);
          LOG(LL_INFO, ("PM2.5 %i PM10 %i", pm25, pm10));
          pm25_accum += 100*pm25;
          pm10_accum += 100*pm10;
          num_measurements++;
        }
        else if (buf[1] == 0xc5) { //measurement
//          LOG(LL_INFO, ("Response %i", buf[2]));
        }
      }
    }
  }
} 

void sds011_sendCommand(uint8_t cmdId, uint8_t data2, uint8_t data3, uint16_t devId) {
  uint8_t buf[19] = {
    0xaa, 0xb4, cmdId, data2, data3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    (devId >> 8) & 0xff, devId & 0xff, 0x00, 0xab
  };
  buf[17] = sds011_checksum(buf, 19);
  size_t len = mgos_uart_write(uart_no, buf, 19);
//  LOG(LL_INFO, ("Wrote %i bytes to SDS011", len));
  if (len < 19) {
    LOG(LL_ERROR, ("Could not write complete SDS011 command"));
  }
}

uint8_t sds011_checksum(uint8_t* buf, uint8_t totalLen) {
  uint16_t sum = 0;
  for (int i = 2; i < totalLen - 2; i++) {
    sum += buf[i];
  }
  return sum & 0xff;
}

void sds011_start(){
  int interval = mgos_sys_config_get_openair_sds011_interval();
  timer_id = mgos_set_timer(interval, MGOS_TIMER_REPEAT, sds011_tick, NULL);
}

void sds011_stop(){
  if (timer_id) {
    mgos_clear_timer(timer_id);
    timer_id = (mgos_timer_id)NULL;
  }
}

// vim: et:sw=2:ts=2

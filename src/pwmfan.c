#include "mgos_gpio.h"
#include "mgos_pwm.h"

#include "pwmfan.h"

static const uint16_t FAN_PWM_FREQ = 10;
static const uint8_t DRIVER_PIN = 5;
static const uint8_t TACHO_PIN = 23;

static volatile uint32_t tacho_count = 0;
static uint16_t lastRpm = 0;
static uint8_t speed = 0;

void tacho_cb(int pin, void *arg) {
  //TODO: Need to debounce short pulses to get good reading
  tacho_count++;
}

void fan_init() {
  mgos_gpio_setup_output(DRIVER_PIN, false);
  mgos_pwm_set(DRIVER_PIN, FAN_PWM_FREQ, 0);
  mgos_gpio_setup_input(TACHO_PIN, MGOS_GPIO_PULL_UP);
  mgos_gpio_set_int_handler(TACHO_PIN, MGOS_GPIO_INT_EDGE_NEG, tacho_cb, NULL);
  mgos_gpio_enable_int(TACHO_PIN);
}

void fan_tick() {

  lastRpm = tacho_count * 1000 / ( mgos_sys_config_get_openair_tick_interval() ) / 2;
  tacho_count = 0;
  if (speed != mgos_sys_config_get_openair_fan_speed()) {
  	speed = mgos_sys_config_get_openair_fan_speed();
	fan_set_power(speed);
  }

}

uint16_t fan_get_rpm() {
  return lastRpm;
}

void fan_set_power(uint8_t power) {
  mgos_pwm_set(DRIVER_PIN, FAN_PWM_FREQ, power / 255.0);
}

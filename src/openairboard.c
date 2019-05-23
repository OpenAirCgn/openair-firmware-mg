#include "openairboard.h"
#include "pwmfan.h"

#define MOD1_EN_PIN 12
#define MOD2_EN_PIN 13

#define LED_PIN 2
#define EXTLED_PIN 4

void openair_init() {
  mgos_gpio_setup_output(
      MOD1_EN_PIN,
      false
  );
  mgos_gpio_setup_output(
      MOD2_EN_PIN, 
      false
  );

  // basic pin config
  mgos_gpio_set_mode(LED_PIN, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_write(LED_PIN, 0);
  mgos_gpio_set_mode(EXTLED_PIN, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_write(EXTLED_PIN, 0);

  // setup fan
  fan_init();
}

void openair_enable_module(uint8_t module, bool on) {
  switch (module) {
    case 1:
      mgos_gpio_write(
        MOD1_EN_PIN,
        on
      );
      break;
    case 2:
      mgos_gpio_write(
        MOD2_EN_PIN,
        on
      );
      break;
    default:
      LOG(LL_ERROR, ("Invalid module index: %i", module));
  }
}

static uint32_t statusPattern = 0x55555555;
static uint8_t statusShift = 0;

void openair_setStatusPattern(uint32_t pattern) {
  statusPattern = pattern;
}

void openair_tick() {
  statusShift = (statusShift+1) & 0x1f;
  bool level = statusPattern & (0x80000000 >> statusShift);
  mgos_gpio_write(LED_PIN, level);
  mgos_gpio_write(EXTLED_PIN, level);

  fan_tick();
}
// vim: et:sw=2:ts=2

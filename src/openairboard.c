#include "openairboard.h"

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

void openair_tick() {
  static int level = 1;
  mgos_gpio_write(LED_PIN, level);
  mgos_gpio_write(EXTLED_PIN, level);
  level ^= 1;
}
// vim: et:sw=2:ts=2

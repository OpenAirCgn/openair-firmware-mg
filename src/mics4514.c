#include "mics4514.h"


static mics4514_cb mics_cb;
static mgos_timer_id timer_id = NULL;

static int VRED_PIN;
static int VOX_PIN;
bool mics4514_init( mics4514_cb cb) {
	mics_cb = cb;
	// enable
	int enable_pin = mgos_sys_config_get_openair_mics4514_pin_en();
	// set red and ox pins
	VRED_PIN = mgos_sys_config_get_openair_mics4514_pin_vred();
	VOX_PIN = mgos_sys_config_get_openair_mics4514_pin_vox();

	mgos_gpio_setup_output(enable_pin, true);
	if (!mgos_adc_enable(VRED_PIN) || !mgos_adc_enable(VOX_PIN)) {
		LOG(LL_ERROR, ("error initializing ADC"));
		return false;
	}
	return true;
}

void mics4514_tick(void * arg) {
  int vred = mgos_adc_read_voltage(VRED_PIN);	
  int vox = mgos_adc_read_voltage(VOX_PIN);	
  LOG(LL_DEBUG, ("mics: %d %d", vred, vox));
  mics_cb (vred, vox);
}

void mics4514_start() {
  int interval = mgos_sys_config_get_openair_mics4514_interval();
  timer_id = mgos_set_timer(interval, MGOS_TIMER_REPEAT, mics4514_tick, NULL);
}

void mics4514_stop() {
  if (timer_id) {
    mgos_clear_timer(timer_id);
    timer_id = (mgos_timer_id)NULL;
  }
}

// vim: et:sw=2:ts=2

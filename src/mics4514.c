#include "mics4514.h"


static mics4514_cb mics_cb;
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

void mics4514_tick() {
  int vred = mgos_adc_read_voltage(VRED_PIN);	
  int vox = mgos_adc_read_voltage(VOX_PIN);	
  LOG(LL_DEBUG, ("mics: %d %d", vred, vox));
  mics_cb (vred, vox);
}

// vim: et:sw=2:ts=2

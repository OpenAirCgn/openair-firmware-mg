#ifndef _OPENAIRBOARD_H_
#define _OPENAIRBOARD_H_

#include "mgos.h"

/** call once to setup */
void openair_init();

/** enables or disables an external module interface - enable pin. Note
 * that not all modules respect this signal.
 *
 *	@param module module index (1 or 2)
 *	@param on true to turn on, false to turn off
 */
void openair_enable_module(uint8_t module, bool on);

void openair_tick();

#endif

// vim: et:sw=2:ts=2

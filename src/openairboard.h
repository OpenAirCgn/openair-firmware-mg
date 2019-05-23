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

/** set the LED blink pattern.
@param pattern a 32 bit blink pattern. Shown from MSB to LSB */

void openair_setStatusPattern(uint32_t pattern);

/** these are just example patterns */
#define OA_BLINK_ONCE_PAUSE 0x80000000
#define OA_BLINK_TWICE_PAUSE 0xa000a000
#define OA_BLINK_CONTINUOUSLY 0xaaaaaaaa

#endif

// vim: et:sw=2:ts=2

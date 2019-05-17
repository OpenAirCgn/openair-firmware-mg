#ifndef _QUADSENSE_H_
#define _QUADSENSE_H_

#include "mgos.h"
#include "mgos_i2c.h"

/** initialize quadsense. Call once at startup (calling it again shouldn't hurt...)
*/

void quadsense_init();

/** call on a regular basis */
bool quadsense_tick();

#endif

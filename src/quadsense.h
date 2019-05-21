#ifndef _QUADSENSE_H_
#define _QUADSENSE_H_

#include "mgos.h"


typedef void (*alphasense_cb) (
    int alpha1,
    int alpha2,
    int alpha3,
    int alpha4,
    int alpha5,
    int alpha6,
    int alpha7,
    int alpha8
    );

typedef void (*bme280_cb) (
    uint32_t p_raw,
    float p,
    uint32_t t_raw,
    float t,
    uint32_t h_raw,
    float h
    );

  /** initialize quadsense. Call once at startup (calling it again shouldn't hurt...)
  */

  void quadsense_init( alphasense_cb a_cb, bme280_cb b_cb );

  /** call on a regular basis */
  bool quadsense_tick();

#endif

// vim: et:sw=2:ts=2

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
    int alpha8,
    int ppb1,
    int ppb2,
    int ppb3,
    int ppb4
    );

typedef void (*bme280_cb) (
    uint8_t idx,
    uint32_t p_raw,
    float p,
    uint32_t t_raw,
    float t,
    uint32_t h_raw,
    float h
    );

  /** initialize quadsense. Call once at startup (calling it again shouldn't hurt...)
  */

  bool quadsense_init( alphasense_cb a_cb, bme280_cb b_cb );
  void quadsense_start();
  void quadsense_stop();

  /** call on a regular basis */
  //bool quadsense_tick();

#endif

// vim: et:sw=2:ts=2

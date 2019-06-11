
#pragma once
#include "mgos.h"
#include "mgos_adc.h"


typedef void (*mics4514_cb) (
    int vred,
    int vox
    );

bool mics4514_init( mics4514_cb cb );
void mics4514_tick();

// vim: et:sw=2:ts=2

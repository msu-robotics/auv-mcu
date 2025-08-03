#pragma once
#include "thrusters/thrusterAllocator.h"


struct ThrustersPins {
    uint8_t thr[5];
};

ThrusterAllocator* createThrusterAllocator();

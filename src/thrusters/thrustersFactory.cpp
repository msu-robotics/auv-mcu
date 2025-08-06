#include "utils.h"
#include "thrusters/thrustersFactory.h"
#include "thrusters/thruster.h"


ThrusterAllocator* createThrusterAllocator() {

    ThrustersPins pins = {{21, 22, 19, 18, 5}};

    Thruster* thrusters[5];
    Thruster* thruster_ptrs[5];
    for (int i = 0; i < 5; ++i) {
        thrusters[i] = new Thruster(pins.thr[i], i);
        // thruster_ptrs[i] = &thrusters[i];
    }
    std::vector<Thruster*> thruster_vec(thrusters, thrusters + 5);

    char buffer[70];
    sprintf(buffer, "ℹ️  Initialized %d thrusters on pins: %d %d %d %d %d",
            5, pins.thr[0], pins.thr[1], pins.thr[2], pins.thr[3], pins.thr[4]);
    uartDebug(buffer);

    auto* allocator = new ThrusterAllocator(6);

    allocator->setThrusters(thruster_vec);

    allocator->setAllocationMatrix({
	//  Fx.      Fy.    Fz.    τx.    τy.    τz.
        { 1.00,  0.00,  0.00,  0.00,  0.00,  1.00 },  // Thruster 1
        { 0.00,  0.00,  1.00,  1.00,  0.00,  0.00 },  // Thruster 2
        { 0.00,  0.00,  1.00, -1.00,  0.00,  0.00 },  // Thruster 3
        {-0.02, -0.98,  0.00,  0.00,  0.00,  0.60 },  // Thruster 4
		{ 0.02,  0.98,  0.00,  0.00,  0.00, -0.60 },  // Thruster 5
    });
	allocator->setCorrectionFactors({1.0, 1.0, 1.0, 1.0, 1.0, 1.0});
    allocator->setReverseFlags({false, false, false, false, false});

    for (int i = 0; i < 5; ++i) {
        thrusters[i]->setup();
    }
    return allocator;
}



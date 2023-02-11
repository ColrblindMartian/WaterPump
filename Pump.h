// Pump.h

#ifndef _PUMP_H
#define _PUMP_H

// initial pin setup for the water pump
void setupPump();

// returns true if the tank reached the upper limit
// or the max inactive timeout was reached
bool TankFull(unsigned long ulLastPumpAction = 0);

// checks the water level and turns on the pump if needed.
// this function will block as long as the pump is enabled.
// if the pump was enabled \param ulLastPumpAction will be set to the current time
void pumpControlLoop(unsigned long& ulLastPumpAction);

#endif

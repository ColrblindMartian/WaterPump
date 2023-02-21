// DeepSleep.h

#ifndef _DEEPSLEEP_h
#define _DEEPSLEEP_h

// send the controller into deep sleep
void DeepSleep(int lMinutes);

// returnes millis() since first boot.
// there is actually a complete esp32 timer library to handle all this,
// but this is a faster fix than reading into that :)
unsigned long millisReal();

#endif
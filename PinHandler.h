// PinHandler.h

#ifndef _PINHANDLER_H
#define _PINHANDLER_H

enum class EPin : int
{
  eWaterSensorUpper = 23,
  eWaterSensorLower = 22,
  eSensorOutUpper   = 19,
  eSensorOutLower   = 18,
  ePumpOut          = 0,

};

// just a small class to make sure we don't use pins multiple times across classes
// constexpr int getPin(EPin pin) // constexpr does not work with switch in arduino ;(
// {
//   // \todo define pins
//   switch(pin)
//   {
//     case EPin::ePumpUpperSensor: return -1;
//     case EPin::ePumpLowerSensor: return -1;
//     case EPin::ePumpOut:         return -1;
//   }
//   return -1;
// }

#endif

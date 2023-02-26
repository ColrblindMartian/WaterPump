// PinHandler.h

#ifndef _PINHANDLER_H
#define _PINHANDLER_H

enum class EPin : int
{
  eWaterSensorUpper = 22,
  eSensorOutUpper   = 23,
  eWaterSensorLower = 18,
  eSensorOutLower   = 19,
  ePumpOut          = 5,

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

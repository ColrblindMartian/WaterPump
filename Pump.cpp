#include "Pump.h"

#include "Arduino.h"
#include "PinHandler.h"
#include "MailHandler.h"
#include "DeepSleep.h"

void setupPump()
{
  pinMode((uint8_t)EPin::eWaterSensorUpper, INPUT_PULLDOWN);
  pinMode((uint8_t)EPin::eWaterSensorLower, INPUT_PULLDOWN);

  // sensors are not directly coonected to 3,3V
  // to make sure no power loss occures during deep sleep
  pinMode((uint8_t)EPin::eSensorOutUpper, OUTPUT);
  pinMode((uint8_t)EPin::eSensorOutLower, OUTPUT);
  digitalWrite((uint8_t)EPin::eSensorOutUpper, HIGH);
  digitalWrite((uint8_t)EPin::eSensorOutLower, HIGH);

  pinMode((uint8_t)EPin::ePumpOut, OUTPUT);
  delay(50);
}

// max time the pump can be stay off, if we are over this time
// we should turn on the pump because maybe the sensors are damaged
constexpr unsigned long getMaxInactionTimoutMS()
{
  // 5 days
  return 5 * 24 * 60 * 60 * 1000;
}

constexpr unsigned long getMaxPumpingTimoutMS()
{
  // 10 minutes
  return 10 * 60 * 1000;
}

bool readSensor(EPin sensor)
{
  // invalid
  if(sensor != EPin::eWaterSensorLower && sensor != EPin::eWaterSensorUpper)
    return false;

  // I don't know how stable the sensors are so the signal is only active if all measured values are high
  bool bSensorActive = true;
  for(int i = 0 ; i <= 5; ++i)
  {
    if(i!=0)
      delay(10);
    bSensorActive &= digitalRead((uint8_t)sensor);
  }
  //char log[20];
  //sprintf(log, "Sensor %i is %s",(uint8_t)sensor, bSensorActive?"high":"low");
  //Serial.println(log);
  return bSensorActive;
}

bool TankFull(unsigned long ulLastPumpAction)
{
  // upper and lower sensors are normaly closed for safety reasons
  bool bTankFull = !readSensor(EPin::eWaterSensorUpper);
  bool bTankEmpty = readSensor(EPin::eWaterSensorLower);

  if(bTankFull)
    return true;

  // if upper sensor says no water, but lower sensor says yes turn the pump on after max inactive time to be sure,
  // maybe upper sensor was damaged?
  // if both sensors are damaged than all hope is lost :)
  if(!bTankEmpty && ulLastPumpAction != 0 &&
     millisReal() - ulLastPumpAction > getMaxInactionTimoutMS())
  {
    Serial.println("Max inactive timeout reached - enable pump");
    sendMail("WaterPump - Timeout", "Inacitve Timeout reached", "Pump will be activated<b>check Sensors!!");
    return true;
  }  
  return false;
}

bool TankEmpty()
{
  // lower sensor is normally closed for safety
  // is sensor is active -> water level is below sensor -> empty
  return readSensor(EPin::eWaterSensorLower);
}

void togglePump(bool bEnable)
{
  digitalWrite((uint8_t)EPin::ePumpOut, bEnable);
}

void pumpControlLoop(unsigned long& ulLastPumpAction)
{
  if(!TankFull(ulLastPumpAction))
    return;

  Serial.println("Tank full -> start emptying");
  sendMail("WaterPump - Tank full", "Tank full", "Pump will be activated");

  // if this is true there has to be a sensor failure
  static bool bDoNotTrustSensor = false;
  {
  bool bDoNotTrustSensorOld = bDoNotTrustSensor;
  bDoNotTrustSensor = TankFull() && TankEmpty();
  // to reduce spam
  if(bDoNotTrustSensor && !bDoNotTrustSensorOld)
    sendMail("WaterPump - Sensor failure", "Sensor failure", "invalid sensor states, check hardware");
  }

  unsigned long startTime = millis();
  togglePump(true);

  bool bSafetySwitchOccured = false;
  do 
  {
    // check if pump is active for too long - sensor failure?
    if(millis() - startTime > getMaxPumpingTimoutMS())
    {
      bSafetySwitchOccured = true;
      break;
    }

    delay(500);
    sendBufferedMails();

    // if we do not trust the sensor we wait for the timeout inside to keep it from switching off instantly
  }while(!TankEmpty() || bDoNotTrustSensor); 
  
  togglePump(false);
  ulLastPumpAction = millisReal();

  if(bSafetySwitchOccured)
  {
    Serial.println("Switching pump off after lower sensor did not get active after 30m!!");
    if(bDoNotTrustSensor)
    {
      sendMail("WaterPump - Pump Off with Invalid Sensor", "Invalid sensor state", "Pump was operated without sensor because of an invalid sensor state<br>"
      "Check hardware");
    }
    else if(TankFull())
    {
      sendMail("WaterPump - Hardware FAILURE", "Emergency switch off", "Pump was switched off automatically,<br>"
      "tank full sensor is still hight!! Sensor or pump failure!");
    }
    else
    {
      sendMail("WaterPump - Emergency Switch OFF", "Emergency switch off", "Pump was switched off automatically,<br>"
      "because the tank empty sensor did not get active after 30 minutes!! Check tank & hardware");
    }
  }
  else
  {
    Serial.println("Tank empty -> switch off pump");
    sendMail("WaterPump - Tank empty", "Tank empty", "Pump will be switched off.");
  }
}

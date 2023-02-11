/*
 Name:    WaterPump.ino
 Created:  1/27/2023 8:07:51 PM
 Author:  Sebastian Aznaid
*/

//#include "MailNotification.h"
#include "MailHandler.h"
#include "DeepSleep.h"
#include "Pump.h"

// will be saved on the rtc controller and 
// will not be lost between deep sleeps
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR unsigned long ulLastPumpAction = 0;

constexpr int lPeriodicSleepTimeMinutes = 60;

// the setup function runs once when you press reset or power the board
void setup() 
{
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  setupPump();

  setupTime();
}

// the loop function runs over and over again until power down or reset
void loop()
{
  pumpControlLoop(ulLastPumpAction);
  if(sendBufferedMails())
    DeepSleep(lPeriodicSleepTimeMinutes);

  // error sending mails? wait a minute
  Serial.println("sleeping 60 sec");
  sleep(60);
}

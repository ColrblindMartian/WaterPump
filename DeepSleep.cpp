#include "DeepSleep.h"

#include "Arduino.h"

// resumed millis, will be written before we enter deep sleep
RTC_DATA_ATTR unsigned long ulResumedMillis = 0;

constexpr uint64_t MinToMicroSeconds(int lMinutes)
{
  return lMinutes * 1000000ULL * 60ULL;
}

constexpr uint64_t MinToMilliSeconds(int lMinutes)
{
  return lMinutes * 1000ULL * 60ULL;
}

void DeepSleep(int lMinutes)
{
  Serial.println("Sending into deep sleep for " + String(lMinutes) + " minutes");
  delay(1000);

  // add sleep time
  ulResumedMillis = millisReal() + MinToMilliSeconds(lMinutes);

  /*
  First we configure the wake up source
  */
  esp_sleep_enable_timer_wakeup(MinToMicroSeconds(lMinutes));

  /*
  Next we decide what all peripherals to shut down/keep on
  By default, ESP32 will automatically power down the peripherals
  not needed by the wakeup source, but if you want to be a poweruser
  this is for you. Read in detail at the API docs
  http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  Left the line commented as an example of how to configure peripherals.
  The line below turns off all RTC peripherals in deep sleep.
  */
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

  /*
  Now that we have setup a wake cause and if needed setup the
  peripherals state in deep sleep, we can now start going to
  deep sleep.
  In the case that no wake up sources were provided but deep
  sleep was started, it will sleep forever unless hardware
  reset occurs.
  */
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
}

unsigned long millisReal()
{
  return millis() + ulResumedMillis;
}

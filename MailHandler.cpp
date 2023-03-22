#include "MailHandler.h"

#include "Credentials.h"

// use version 1.6.4!!
// newer versions throw exceptions when wifi is unstable
#include <ESP_Mail_Client.h>
#include "time.h"

#include "DeepSleep.h"

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* these defines have to be set: */
// wifi
//     #define WIFI_SSID
//     #define WIFI_PASSWORD
// mail
//     #define AUTHOR_EMAIL
//     #define AUTHOR_PASSWORD
//     #define RECIPIENT_EMAIL

/* time server */
constexpr char* ntpServer = "pool.ntp.org";
constexpr long  gmtOffset_sec = 3600;
constexpr int   daylightOffset_sec = 3600;

/* The SMTP Session object used for Email sending */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

void setupTime()
{
  // nothing more to be done here, wifi connection can be set up later
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

// wifi and mail setup
bool wifiSetup()
{
  if(WiFi.isConnected())
    return true;

  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(200);
    if((millis() - startTime) > (10 * 1000))
    {
      Serial.println("Wifi connection timeout");
      return false;
    }
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  static bool bOneTimeSmtpSetup = false;
  if(!bOneTimeSmtpSetup)
  {
    /** Enable the debug via Serial port
      * none debug or 0
      * basic debug or 1
    */
    smtp.debug(1);

    /* Set the callback function to get the sending results */
    smtp.callback(smtpCallback);

    bOneTimeSmtpSetup = true;
  }

  return true;
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status) {
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()) {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}

struct MailMessageEntry
{
  uint lRemainingTries = 0;
  SMTP_Message oMsg;

  bool isValid() { return lRemainingTries > 0; }
  void setValid() { lRemainingTries = 3; }
};

constexpr int lMaxMessages = 4;
// points to the oldes msg entry
int lCurrentIx = 0;
MailMessageEntry lstMessages[lMaxMessages];

void addMail(const SMTP_Message& msg)
{
  if(lstMessages[lCurrentIx].isValid())
  {
    Serial.println("Old mail message found... overwrite with new one");
  }
  lstMessages[lCurrentIx].oMsg = msg;
  lstMessages[lCurrentIx].setValid();

  lCurrentIx++;
  if(lCurrentIx >= lMaxMessages)
    lCurrentIx = 0;
}

void removeMail(int ix)
{
  lstMessages[ix].lRemainingTries = 0;
}

bool mailListEmpty()
{
  for(MailMessageEntry& entry : lstMessages)
  {
    if(entry.isValid())
      return false;
  }  
  return true;
}

bool sendBufferedMails()
{
  if(mailListEmpty())
    return true;

  if(!wifiSetup())
    return false;

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return false;

  bool bError = false;
  int ix = 0;
  for(int i = lCurrentIx; i < lCurrentIx + lMaxMessages; i++)
  {
    ix = (i >= lMaxMessages) ? i - lMaxMessages : i;

    if(!lstMessages[ix].isValid())
      continue;

    if (!MailClient.sendMail(&smtp, &lstMessages[ix].oMsg, false))
    {
      Serial.println("Error sending Email, " + smtp.errorReason());
      lstMessages[ix].lRemainingTries--;
      bError = true;
      break;
    }
    removeMail(ix); 
  }
  lCurrentIx = ix;
  return !bError;
}

void sendMail(const String& strSubject,
              const String& strHeading,
              const String& strText)

{
  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "WaterPump";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = strSubject;
  message.addRecipient("RobotMaster", RECIPIENT_EMAIL);
  

  String strTime;
  strTime += "MillisReal: " + String(millisReal()) + "<br>";
  
  {
  wifiSetup();
  tm timeinfo;
  //init and get the time
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    strTime += "Error getting time";
  }
  else
  {
    strTime += asctime(&timeinfo);
  }
  }

  /*Send HTML message*/
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>" + strHeading + "</h1><p> "+ strText +"<br><br>" + strTime + "</p></div>";
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  addMail(message);
  sendBufferedMails();
}

// Mailhandler.h

#ifndef _MAILHANDLER_h
#define _MAILHANDLER_h

#include <Arduino.h>

void setupTime();

// this will send a mail, if not successfull the mail will be added to the buffer and
// send at a later date
void sendMail(const String& strSubject,
              const String& strHeading,
              const String& strText);

// returns true if no mail is in the buffer -> all mails have been sent
bool mailListEmpty();
// send mails that are saved in the buffer
bool sendBufferedMails();

#endif
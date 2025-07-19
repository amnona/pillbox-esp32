/*
  Rui Santos
  Complete project details at:
   - ESP32: https://RandomNerdTutorials.com/esp32-send-email-smtp-server-arduino-ide/
   - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-send-email-smtp-server-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Example adapted from: https://github.com/mobizt/ESP-Mail-Client
*/

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h>
#include <HTTPClient.h>
#include <time.h>

#include "secrets.h" // 

/* time management */
time_t now;
char strftime_buf[64];
struct tm timeinfo;
struct tm lasttimeinfo;
struct tm lastEmailTimeInfo;
struct tm lastKeepAliveTime;

/* Pillbox sensor and leds */
#define LID_OPEN HIGH
#define LID_CLOSED LOW
#define LID_SENSOR_PIN 15
#define LED_GREEN_PIN 12
#define LED_RED_PIN 14
#define LED_BLUE_PIN 13

int ledState=0;
int oldLidState = LID_CLOSED;

int lidOpenedToday = false;

// Timezone and NTP server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600 * 2; // UTC+2 (Israel Standard Time, for example)
const int   daylightOffset_sec = 3600; // add 1 hour if daylight saving time applies

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

void setup(){
  /* setup the serial */
  Serial.begin(115200);
  Serial.println();

  /* initialize the pillbox input/output pins*/
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(LID_SENSOR_PIN, INPUT_PULLUP);

  /* connect to wifi */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /*  Set the network reconnection option */
  MailClient.networkReconnect(true);

  /** Enable the debug via Serial port
   * 0 for no debugging
   * 1 for basic level debugging
   *
   * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
   */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  // /* Declare the Session_Config for user defined session credentials */
  // Session_Config config;

  // /* Set the session config */
  // config.server.host_name = SMTP_HOST;
  // config.server.port = SMTP_PORT;
  // config.login.email = AUTHOR_EMAIL;
  // config.login.password = AUTHOR_PASSWORD;
  // config.login.user_domain = "";

  // /*
  // Set the NTP config time
  // For times east of the Prime Meridian use 0-12
  // For times west of the Prime Meridian add 12 to the offset.
  // Ex. American/Denver GMT would be -6. 6 + 12 = 18
  // See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  // */
  // config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  // config.time.gmt_offset = 2;
  // config.time.day_light_offset = 60;

  // /* Declare the message class */
  // SMTP_Message message;

  // /* Set the message headers */
  // message.sender.name = F("PillBox");
  // message.sender.email = AUTHOR_EMAIL;
  // message.subject = F("PillBox started");
  // message.addRecipient(F("Amnon"), RECIPIENT_EMAIL);
    
  // /*Send HTML message*/
  // /*String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Hello World!</h1><p>- Sent from ESP board</p></div>";
  // message.html.content = htmlMsg.c_str();
  // message.html.content = htmlMsg.c_str();
  // message.text.charSet = "us-ascii";
  // message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;*/

   
  // //Send raw text message
  // String textMsg = "Pillbox started";
  // message.text.content = textMsg.c_str();
  // message.text.charSet = "us-ascii";
  // message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  // message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  // message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  // // Configure time via NTP
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // /* Connect to the server */
  // if (!smtp.connect(&config)){
  //   ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  //   SendGet("debug","Error connecting to smtp for pillbox started mail");
  //   return;
  // }

  // if (!smtp.isLoggedIn()){
  //   Serial.println("\nNot yet logged in.");
  // }
  // else{
  //   if (smtp.isAuthenticated())
  //     Serial.println("\nSuccessfully logged in.");
  //   else
  //     Serial.println("\nConnected with no Auth.");
  // }

  // // SendGet("debug/sent_start_email");
  // SendGet("debug","sent_start_email");

  // /* Start sending Email and close the session */
  // if (!MailClient.sendMail(&smtp, &message)) {
  //   ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  //   SendGet("debug","Error sending for pillbox started mail");
  // }

  SendPillMail("Pillbox started","I am ready now");

  // // Configure time via NTP
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // time(&now);
  // localtime_r(&now, &timeinfo);
  // localtime_r(&now, &lasttimeinfo);
  // localtime_r(&now, &lastEmailTimeInfo);
  // localtime_r(&now, &lastKeepAliveTime);
  getLocalTime(&timeinfo);
  getLocalTime(&lasttimeinfo);
  getLocalTime(&lastEmailTimeInfo);
  getLocalTime(&lastKeepAliveTime);

  SendGet("debug","started_pillbox");
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}


void SendPillMail(String subject, String content) {
  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 2;
  config.time.day_light_offset = 60;

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("PillBox");
  message.sender.email = AUTHOR_EMAIL;
  // message.subject = F("PillBox started");
  message.subject = subject.c_str();
  message.addRecipient(F("Amnon"), RECIPIENT_EMAIL);
    
  /*Send HTML message*/
  /*String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Hello World!</h1><p>- Sent from ESP board</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;*/

   
  //Send raw text message
  // String textMsg = "Pillbox started";
  // message.text.content = textMsg.c_str();
  message.text.content = content.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  // Configure time via NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  /* Connect to the server */
  if (!smtp.connect(&config)){
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    SendGet("debug","Error connecting to smtp for pillbox started mail");
    return;
  }

  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  }
  else{
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  // SendGet("debug/sent_start_email");
  SendGet("debug","sent_email");

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message)) {
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    SendGet("debug","Error sending for pillbox started mail");
  }


  // Session_Config config;

  //   SendGet("debug","preparing to send reminder email");
  // /* Set the session config */
  // config.server.host_name = SMTP_HOST;
  // config.server.port = SMTP_PORT;
  // config.login.email = AUTHOR_EMAIL;
  // config.login.password = AUTHOR_PASSWORD;
  // config.login.user_domain = "";

  // // /*
  // // Set the NTP config time
  // // For times east of the Prime Meridian use 0-12
  // // For times west of the Prime Meridian add 12 to the offset.
  // // Ex. American/Denver GMT would be -6. 6 + 12 = 18
  // // See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  // // */
  // config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  // config.time.gmt_offset = 2;
  // config.time.day_light_offset = 60;

  // /* Declare the message class */
  // SMTP_Message message;

  // /* Set the message headers */
  // message.sender.name = F("PillBox");
  // message.sender.email = AUTHOR_EMAIL;
  // message.subject = F("Did you Forget to take your pill");
  // message.addRecipient(F("Amnon"), RECIPIENT_EMAIL);
   
  // //Send raw text message
  // String textMsg = "Amnon loves you moremoremoremoremoremoremoremoremoremore";
  // message.text.content = textMsg.c_str();
  // message.text.charSet = "us-ascii";
  // message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  // message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  // message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  // /* Connect to the server */
  // if (!smtp.connect(&config)){
  //   ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  //   SendGet("debug","failed to connect for reminder email");
  //   return;
  // }

  // if (!smtp.isLoggedIn()){
  //   Serial.println("\nNot yet logged in.");
  // }
  // else{
  //   if (smtp.isAuthenticated())
  //     Serial.println("\nSuccessfully logged in.");
  //   else
  //     Serial.println("\nConnected with no Auth.");
  // }

  // /* Start sending Email and close the session */
  // if (!MailClient.sendMail(&smtp, &message)) {
  //   ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  //    SendGet("debug","Error sending reminder email");
  //    return;
  // }
  Serial.println("Mail sent");
  SendGet("debug","finised_send_mail");
}


void SendGet(String address, String msg) {
  String serverName = "http://servo.local:5000";
  
  HTTPClient http;
  String serverPath = serverName + "/" + address;
  String newMsg;
  char timeStringBuff[30];

  getLocalTime(&timeinfo);
  sprintf(timeStringBuff, "%02d-%02d-%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  Serial.println(msg);

  newMsg = serverPath+"/" + String(msg)+'_'+String(timeStringBuff) + "__lid_opened_today=" + String(lidOpenedToday);
  for (char& c : newMsg) {
      if (c == ' ') {
          c = '_';
      }
  }
  // http.begin(serverPath.c_str());
  Serial.print("Sending get to: ");
  Serial.println(serverPath);
  http.begin(newMsg.c_str());
  Serial.println(newMsg.c_str());
  int httpResponseCode = http.GET();
  if (httpResponseCode>0) {
    Serial.print("Send Get: ");
    Serial.println(serverPath);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("http get Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}



void loop() {
  int lidState;
  String lidStateStr;
  int tmpLid1;
  int tmpLid2;
  int tmpLid3;
  int tmpLid4;

  lidState = digitalRead(LID_SENSOR_PIN);
  lidStateStr = String(lidState);
  // SendGet("debug","Current lid state: "+lidStateStr);
  if (lidState != oldLidState) {
    Serial.println("Changed.");
    SendGet("debug","lid_state_changed");

    lidStateStr = String(lidState);
    SendGet("debug","new_state="+lidStateStr);

    // Serial.println("Changed. New state %d", lidState);
    delay(25);
    tmpLid1 = digitalRead(LID_SENSOR_PIN);
    delay(25);
    tmpLid2 = digitalRead(LID_SENSOR_PIN);
    // delay(25);
    // tmpLid3 = digitalRead(LID_SENSOR_PIN);
    // delay(25);
    // tmpLid4 = digitalRead(LID_SENSOR_PIN);

    // if ((lidState == tmpLid1) && (tmpLid1==tmpLid2) && (tmpLid2==tmpLid3) && (tmpLid3==tmpLid4)) {
    if ((lidState == tmpLid1) && (tmpLid1==tmpLid2)) {
      lidStateStr = String(lidState);
      SendGet("debug","new_state_detected="+lidStateStr);
      if (lidState == LID_OPEN) {
        SendGet("debug","lid_open");
      } else {
        SendGet("debug","lid_close");
      }
    } else {
      lidStateStr = String(lidState)+","+String(tmpLid1)+","+String(tmpLid2)+","+String(tmpLid3)+","+String(tmpLid4);
      SendGet("debug","flicker_lid_state="+lidStateStr);
      lidState = oldLidState;
    }
    oldLidState = lidState;
  }

if (lidState == LID_OPEN) {
    if (!lidOpenedToday) {
      SendGet("debug","lid_opened_today");
    }
    lidOpenedToday = true;
  } else {
  }

/*
  oldLidState = lidState;
  ledState = 1 - ledState;
  digitalWrite(LED_RED_PIN, ledState);
*/

  // time(&now);
  // localtime_r(&now, &timeinfo);
  getLocalTime(&timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  // Serial.println(strftime_buf);

  /* if a new day - reset the alarm */
  // if (lasttimeinfo.tm_hour > timeinfo.tm_hour) {
  if (lasttimeinfo.tm_mday != timeinfo.tm_mday) {
    Serial.println("New day - resetting lidopen");
    SendGet("debug","new_day_resetting_lidopen");
    lidOpenedToday = false;
  }
  
  if (!lidOpenedToday) {
    if (timeinfo.tm_hour >= 11) {
      // Serial.println("Need to take your pill");
      if (lastEmailTimeInfo.tm_hour != timeinfo.tm_hour) {
        Serial.println("sending reminder email");
        SendGet("debug","sending_reminder_email");
        SendPillMail("PillBox reminder","Did you forget to take your pill?\nAmnon loves you moremoremoremoremoremoremoremore");
        SendGet("debug","reminder_email_sent");
        lastEmailTimeInfo = timeinfo;
      }
    }
  }

  if (lastKeepAliveTime.tm_min != timeinfo.tm_min) {
    SendGet("debug","keep_alive");
    lastKeepAliveTime = timeinfo;
  }

  digitalWrite(LED_GREEN_PIN, lidOpenedToday);
  digitalWrite(LED_RED_PIN, !lidOpenedToday);
  // digitalWrite(LED_GREEN_PIN, 1);
  // digitalWrite(LED_RED_PIN, 1);
  digitalWrite(LED_BLUE_PIN, lidState);

  lasttimeinfo = timeinfo;

  delay(200);
}


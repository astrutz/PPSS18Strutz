#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>;

#if defined (ESP8266)
GxEPD2_3C < GxEPD2_420c, GxEPD2_420c::HEIGHT / 2 > display(GxEPD2_420c(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4));
#endif

#if defined(ESP32)
#endif

#if defined(_BOARD_GENERIC_STM32F103C_H_)
#define MAX_DISPAY_BUFFER_SIZE 15000ul // ~15k is a good compromise
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#define MAX_HEIGHT_3C(EPD) (EPD::HEIGHT <= (MAX_DISPAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#endif

#if defined(__AVR)
#define MAX_DISPAY_BUFFER_SIZE 800 // 
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#define MAX_HEIGHT_3C(EPD) (EPD::HEIGHT <= (MAX_DISPAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#define MAX_HEIGHT_3C(EPD) (EPD::HEIGHT <= (MAX_DISPAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#endif

#ifndef STASSID
//#define STASSID "WG-Lan"
//#define STAPSK  "52489315989173508174"
# define STASSID "kernarea.de/BYOD"
#define STAPSK "4m0b!l35"
#endif

#include "ESP8266HTTPClient.h"
#include "ESP8266WiFi.h"

const char* ssid = "kernarea.de/BYOD";
const char* password =  "4m0b!l35";
const char* abbreviation = "";
const byte interruptPin = 12;
const int cardID = 1234;
int pressed = false;
int Led = 2; //LED_BUILTIN = GPIO2
StaticJsonDocument<2048> doc;
void drawStory(const char &name, const char &abbr, const char &assignee);
void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup ()
{
  Serial.begin(115200);
  wdt_enable(WDTO_1S);   // Watchdog auf 1 s stellen
  pinMode (interruptPin, INPUT_PULLUP);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  attachInterrupt(digitalPinToInterrupt(interruptPin), test, RISING);
  HTTPClient http;

  http.begin("http://jiracardserver.herokuapp.com/card/issue/1234");
  //http.addHeader("Content-Type", "text/plain");

  int httpCode = http.GET();
  if (httpCode > 0) {
    deserializeJson(doc, http.getString());
    abbreviation = doc["abbreviation"];
  }
  Serial.println("Setup done");
}

void loop ()
{
  if (pressed == true) {
    if (WiFi.status() == WL_CONNECTED) {

      HTTPClient http;

      http.begin("http://jiracardserver.herokuapp.com/dailyStatus");
      //http.addHeader("Content-Type", "text/plain");

      int httpCode = http.GET();

      if (httpCode > 0) {

        String response = http.getString();
        if (response == "null") {
          char url[128];
          strcpy(url, "http://jiracardserver.herokuapp.com/card/issue/");
          strcat(url, cardID);
          http.begin(url);

          int httpResponseCode = http.GET();

          if (httpResponseCode > 0) {

            String response = http.getString();
            deserializeJson(doc, http.getString());
            const char* storyName = doc["name"];
            abbreviation = doc["abbreviation"];
            const char* storyAssignee = doc["assignee"];
            Serial.println(storyName);
            drawStory(storyName, abbreviation, storyAssignee);
            Serial.println("Story written on display");

          } else {

            Serial.print("Error on sending GET Request: ");
            Serial.println(httpResponseCode);
          }

        } else {
          Serial.println("Logged in user is " + response);
          if (abbreviation != "") {
            char url[128];
            strcpy(url, "http://jiracardserver.herokuapp.com/status/");
            strcat(url, abbreviation);
            http.begin(url);
            int httpResponseCode = http.PUT("");
            if (httpResponseCode > 0) {
              Serial.println("Status changed");
            } else {

            }
          } else {
            Serial.println("Somebody is logged in, but no Story here to change Status");
          }
        }

      } else {

        Serial.print("Error on sending GET Request: ");
        Serial.println(httpCode);

      }

      http.end();

    } else {
      Serial.println("Error in WiFi connection");
    }
    resetFunc();
  }
  pressed = false;

}


void drawStory(const char* text, const char* abbr, const char* assignee)
{
  display.init(115200);
  display.setFont(&FreeSans18pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = tbw;
  uint16_t y = tbh;
  display.setFullWindow();
  display.firstPage();
  display.fillScreen(GxEPD_WHITE);
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x + 50, 0);
    display.println(abbr);
    display.setFont(&FreeSansBold18pt7b);
    display.println(text);
    display.setCursor(x + 50, display.height() - 50);
    display.setFont(&FreeSans12pt7b);
    display.print(assignee);
  }
  while (display.nextPage());
  display.powerOff();
  Serial.println("Display write completed");
  //display.hibernate();
}

void test()
{
  Serial.println("Button pressed");
  pressed = true;
}

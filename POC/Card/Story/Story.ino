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

const char* ssid = "WG-Lan";
const char* password = "52489315989173508174";
//const char* ssid = "kernarea.de/BYOD";
//const char* password = "4m0b!l35";
const char* abbreviation = "";
const byte interruptPin = 12;
const char* cardID = "1234";
int pressed = false;
int Led = 2; //LED_BUILTIN = GPIO2
StaticJsonDocument<2048> doc;
void drawStory(const char &title, const char &abbr, const char &assignee);
void ICACHE_RAM_ATTR buttonClick();

void setup ()
{
  Serial.begin(115200);
  display.init(115200);
  display.hibernate();
  pinMode (interruptPin, INPUT_PULLUP);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  attachInterrupt(digitalPinToInterrupt(interruptPin), buttonClick, RISING);
  HTTPClient http;

  http.begin("http://jiracardserver.herokuapp.com/card/issue/1234");

  int httpCode = http.GET();
  if (httpCode > 0) {
    deserializeJson(doc, http.getString());
    abbreviation = doc["abbreviation"];
  }
  Serial.println("Setup completed");
}

void loop ()
{
  if (pressed == true) {
    if (WiFi.status() == WL_CONNECTED) {

      HTTPClient http;
      http.begin("http://jiracardserver.herokuapp.com/dailyStatus");
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
            drawStory(storyName, abbreviation, storyAssignee);

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
  }
  pressed = false;

}

void drawStory(const char* title, const char* abbr, const char* assignee)
{
  display.setFont(&FreeSans18pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(title, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = tbw;
  uint16_t y = tbh;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x + 50, 0);
    display.println(abbr);
    display.setFont(&FreeSansBold18pt7b);
    display.println(title);
    display.setCursor(x + 50, display.height() - 50);
    display.setFont(&FreeSans12pt7b);
    display.print(assignee);
  }
  while (display.nextPage());
  display.powerOff();
  Serial.println("Display write completed");
}

void ICACHE_RAM_ATTR buttonClick()
{
  Serial.println("Button pressed");
  pressed = true;
}

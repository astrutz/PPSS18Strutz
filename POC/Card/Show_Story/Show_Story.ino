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

const char* ssid     = STASSID;
const char* password = STAPSK;
StaticJsonDocument<2048> doc;
void drawStory(const char &name, const char &abbr, const char &assignee);

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) { 
  delay(1000); 
  }
  
  Serial.println("setup done");
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED) { 
 
HTTPClient http;  
 
//http.begin("http://192.168.178.52:3000/issue/AGRV-729");  von zuhause
//http.begin("http://192.168.107.245:3000/issue/PPBA-21"); //von der Arbeit
http.begin("http://jiracardserver.herokuapp.com/issue/PPBA-8"); //mocked API von der Arbeit

int httpCode = http.GET(); 
 
if (httpCode > 0) { 

Serial.println(http.getString());
deserializeJson(doc, http.getString());
const char* storyName = doc["name"];
const char* storyAbbr = doc["abbreviation"];
const char* storyAssignee = doc["assignee"];
drawStory(storyName, storyAbbr, storyAssignee);
}
 
http.end();   //Close connection
 
}
 
delay(30000);    //Send a request every 30 seconds
 
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
  display.hibernate();
}

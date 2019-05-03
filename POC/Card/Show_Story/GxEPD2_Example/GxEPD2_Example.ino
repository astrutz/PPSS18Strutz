#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
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
void drawStory(const char &name, const char &desc, const char &abbr);

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) { 
  delay(1000); 
  }
  
  Serial.println("WiFI Connected");
  /*
  display.init(115200);
  helloWorld();
  delay(1000);
  display.powerOff();
  deepSleepTest();
  */
  Serial.println("setup done");
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED) { 
 
HTTPClient http;  
 
//http.begin("http://192.168.178.52:3000/AGRV-729");  von zuhause
http.begin("http://192.168.107.245:3000/PPBA-21"); //von der Arbeit
int httpCode = http.GET(); 
 
if (httpCode > 0) { 
 
deserializeJson(doc, http.getString());
const char* storyName = doc["name"];
const char* storyDesc = doc["description"];
const char* storyAbbr = doc["abbreviation"];
drawStory(storyName, storyDesc, storyAbbr);
}
 
http.end();   //Close connection
 
}
 
delay(30000);    //Send a request every 30 seconds
 
}

const char HelloWorld[] = "Hello Ivan!";

void drawStory(const char* text, const char* desc, const char* abbr)
{
  display.init(115200);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = (display.width() - tbw) / 2;
  uint16_t y = (display.height() + tbh) / 2; // y is base line!
  display.setFullWindow();
  display.firstPage();
  display.fillScreen(GxEPD_WHITE);
  String sum = "";
  sum.concat(abbr);
  sum.concat("\n");
  sum.concat(text);
  sum.concat("\n");
  sum.concat(desc);
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(sum);
  }
  while (display.nextPage());
  display.powerOff();
}

void helloWorld()
{
  //Serial.println("helloWorld");
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = (display.width() - tbw) / 2;
  uint16_t y = (display.height() + tbh) / 2; // y is base line!
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(HelloWorld);
  }
  while (display.nextPage());
  //Serial.println("helloWorld done");
}

void deepSleepTest()
{
  //Serial.println("deepSleepTest");
  const char hibernating[] = "hibernating ...";
  const char wokeup[] = "woke up from deep sleep";
  const char again[] = "hibernating again ...";
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  // center text
  display.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = (display.width() - tbw) / 2;
  uint16_t y = (display.height() + tbh) / 2; // y is base line!
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(hibernating);
  }
  while (display.nextPage());
  display.hibernate();
  delay(5000);
  display.getTextBounds(wokeup, 0, 0, &tbx, &tby, &tbw, &tbh);
  x = (display.width() - tbw) / 2;
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(wokeup);
  }
  while (display.nextPage());
  delay(5000);
  display.getTextBounds(again, 0, 0, &tbx, &tby, &tbw, &tbh);
  x = (display.width() - tbw) / 2;
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(again);
  }
  while (display.nextPage());
  display.hibernate();
  //Serial.println("deepSleepTest done");
}

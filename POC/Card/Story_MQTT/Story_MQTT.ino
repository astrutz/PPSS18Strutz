#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const unsigned char bitmap [] PROGMEM =
{
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F,
  0x1F, 0x0F, 0x87, 0xF7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x0F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x7F, 0x7F, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x07,
  0x0F, 0x1F, 0xFE, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x07, 0x0F, 0x3F, 0x3F, 0x07, 0x01, 0x00, 0x00, 0x00,
  0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0x3F, 0x0F, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x0F, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0xC4, 0xF8, 0xF0,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x0F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF1,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF8,
  0xFF, 0xFE, 0xF8, 0xFC, 0xFF, 0xFF, 0xFF, 0xF3, 0xC0, 0xFE,
  0xF0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFE, 0xFC, 0xF8, 0xF8, 0xF0, 0xF0, 0xF3, 0xE7, 0xEF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7,
  0xF0, 0xF8, 0xF8, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/**************************** CONFIG ZONE ***********************/
const char* ssid = "WG-Lan";
const char* password = "52489315989173508174";
const char* mqtt_server = "192.168.178.52";
const char* card_id = "1234545";
/****************************************************************/

#if defined (ESP8266)
GxEPD2_3C < GxEPD2_420c, GxEPD2_420c::HEIGHT / 2 > display(GxEPD2_420c(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4));
#endif

WiFiClient espClient;
PubSubClient client(espClient);
HTTPClient http;
StaticJsonDocument<2048> doc;
void drawStory(const char &abbr, const char &title, int &progress, const char &assignee);
void ICACHE_RAM_ATTR buttonClick();
int pressed = false;
const byte interruptPin = 12;

void setup_wifi() {

  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void drawStory(const char* abbr, const char* title, int progress, const char* assignee)
{
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeSansBold12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(0, 30);
    display.println(abbr);
    display.println();
    display.setFont(&FreeSans18pt7b);
    display.println(title);
    if (progress < 100) {
      display.fillRect(20, 150, progress, 20, GxEPD_BLACK);
      display.drawRect(20 + progress, 150, 100 - progress, 20, GxEPD_BLACK);
    } else {
      display.drawBitmap(20, 150, bitmap, 48, 48, GxEPD_RED);
    }
    display.setFont(&FreeSans12pt7b);
    display.setCursor(0, 280);
    display.println(assignee);
  }
  while (display.nextPage());
  display.hibernate();
  Serial.println("Display write completed");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char url[128];
  strcpy(url, "http://");
  strcat(url, mqtt_server);
  strcat(url, ":3000/issue/");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    char c = char(payload[i]);
    char *pChar = &c;
    strcat(url, pChar);
  }
  Serial.println();
  if (length > 1) {

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {

      String response = http.getString();
      deserializeJson(doc, http.getString());
      const char* rowOne = doc["row1"];
      const char* rowZero = doc["row0"];
      const char* rowThree = doc["row3"];
      int rowTwo = doc["row2"];
      drawStory(rowZero, rowOne, rowTwo, rowThree);

    } else {
      Serial.print("Error on sending GET Request: ");
      Serial.println(httpResponseCode);
    }

  } else {
    display.setFullWindow();
    display.firstPage();
    do
    {
      display.setFont(&FreeSans18pt7b);
      display.fillScreen(GxEPD_WHITE);
      display.setCursor(80, 130);
      display.setTextColor(GxEPD_BLACK);
      display.println("Display ist frei");
      display.setFont(&FreeSans12pt7b);
      display.setCursor(80, 170);
      display.print("Karten-ID: ");
      display.print(card_id);
    }
    while (display.nextPage());
    display.hibernate();
  }


}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(card_id);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  display.init(115200);
  display.hibernate();
  pinMode (interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), buttonClick, RISING);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (pressed == true) {
    char url[128];
    strcpy(url, "http://");
    strcat(url, mqtt_server);
    strcat(url, ":3000/card/");
    strcat(url, card_id);
    http.begin(url);
    int httpResponseCode = http.PUT("");
    if (httpResponseCode > 0) {
      Serial.println("Status changed");
    }
    pressed = false;
  }
}

void ICACHE_RAM_ATTR buttonClick()
{
  pressed = true;
}

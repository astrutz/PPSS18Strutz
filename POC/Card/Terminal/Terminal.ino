#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define RST_PIN  0          // GPIO0 = D3
#define SS_PIN   15         // GPIO15 = D8

const char* ssid = "kernarea.de/BYOD";
const char* password =  "4m0b!l35";
const byte interruptPin = 4; //GPIO4 = D2
boolean pressed = false;

MFRC522 rfid(SS_PIN, RST_PIN);
 
byte tagUID[4];
void ICACHE_RAM_ATTR logOff();

void setup() {
  attachInterrupt(digitalPinToInterrupt(interruptPin), logOff, RISING);
	Serial.begin(115200);		
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Waiting for WiFi Connection..");
  }
  
	SPI.begin();	
	rfid.PCD_Init();	
  delay(5);

  Serial.println("Setup complete");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && pressed == true) {

      HTTPClient http;
      http.begin("http://jiracardserver.herokuapp.com/logoff");
      int httpCode = http.PUT("");

      if (httpCode > 0) {
        Serial.println(http.getString());
      }
  }
  pressed = false;
  
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  if (rfid.uid.uidByte[0] != tagUID[0] || 
    rfid.uid.uidByte[1] != tagUID[1] || 
    rfid.uid.uidByte[2] != tagUID[2] || 
    rfid.uid.uidByte[3] != tagUID[3] ) {

    for (byte i = 0; i < 4; i++) {
      tagUID[i] = rfid.uid.uidByte[i];
    }
    sendUidToServer();
  }
  else Serial.println(F("Card read previously."));

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1(); 
}

void sendUidToServer() {
  unsigned long hex_num;
  hex_num =  tagUID[0] << 24;
  hex_num += tagUID[1] << 16;
  hex_num += tagUID[2] <<  8;
  hex_num += tagUID[3];

  if (WiFi.status() == WL_CONNECTED) {

      HTTPClient http;
      String hexString = String(hex_num);
      char url[128];
      char buf[32];
      hexString.toCharArray(buf, 32);
      strcpy(url, "http://jiracardserver.herokuapp.com/login/");
      strcat(url, buf);
      http.begin(url);

      int httpCode = http.PUT("");

      if (httpCode > 0) {
        Serial.println(http.getString());
      }
  }
}

void ICACHE_RAM_ATTR logOff()
{
  pressed = true;
}

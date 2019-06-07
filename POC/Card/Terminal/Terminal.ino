#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ESP8266HTTPClient.h"
#include "ESP8266WiFi.h"

#define RST_PIN  0          // GPIO0 = D3
#define SS_PIN   15         // GPIO15 = D8

const char* ssid = "kernarea.de/BYOD";
const char* password =  "4m0b!l35";
const byte interruptPin = 4; //GPIO4 = D2
boolean pressed = false;

MFRC522 rfid(SS_PIN, RST_PIN);  // Create MFRC522 instance

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];
void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup() {
  attachInterrupt(digitalPinToInterrupt(interruptPin), logOff, RISING);
	Serial.begin(115200);		// Initialize serial communications with the PC
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  
	SPI.begin();			// Init SPI bus
	rfid.PCD_Init();		// Init MFRC522
  delay(5);

   for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

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
      resetFunc();
  }
  pressed = false;
  
	// Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
    sendUidToServer();
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

}

void sendUidToServer() {
  unsigned long hex_num;
  hex_num =  nuidPICC[0] << 24;
  hex_num += nuidPICC[1] << 16;
  hex_num += nuidPICC[2] <<  8;
  hex_num += nuidPICC[3];

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

void logOff()
{
  pressed = true;
}

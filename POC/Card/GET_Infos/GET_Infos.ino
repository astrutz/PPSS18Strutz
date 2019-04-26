#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>;

#ifndef STASSID
//#define STASSID "WG-Lan"
//#define STAPSK  "52489315989173508174"
# define STASSID "kernarea.de/BYOD"
#define STAPSK "4m0b!l35"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;
StaticJsonDocument<256> doc;

void setup () {
 
Serial.begin(115200);
WiFi.begin(ssid, password);
 
while (WiFi.status() != WL_CONNECTED) { 
delay(1000); 
}
 
}
 
void loop() {
 
if (WiFi.status() == WL_CONNECTED) { 
 
HTTPClient http;  
 
//http.begin("http://192.168.178.52:3000/AGRV-728");  von zuhause
http.begin("http://192.168.107.245:3000/AGRV-728"); //von der Arbeit
int httpCode = http.GET(); 
 
if (httpCode > 0) { 
 
deserializeJson(doc, http.getString());
const char* storyName = doc["name"];
const char* description = doc["description"];
const char* abbreviation = doc["abbreviation"];
const char* assignee = doc["assignee"];
const char* storyStatus = doc["status"];
const char* issuetype = doc["issuetype"];

Serial.println(http.getString());
//Serial.println(storyName);
Serial.println(description);
Serial.println(abbreviation);
Serial.println(assignee);
Serial.println(storyStatus);
Serial.println(issuetype);
 
}
 
http.end();   //Close connection
 
}
 
delay(30000);    //Send a request every 30 seconds
 
}

void writeCard(){
}
}

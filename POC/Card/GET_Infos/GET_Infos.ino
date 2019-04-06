/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>;

#ifndef STASSID
#define STASSID "WG-Lan"
#define STAPSK  "52489315989173508174"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;
StaticJsonDocument<256> data;

void setup () {
 
Serial.begin(115200);
WiFi.begin(ssid, password);
 
while (WiFi.status() != WL_CONNECTED) {
 
delay(1000);
Serial.print("Connecting..");
 
}
 
}
 
void loop() {
 
if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
 
HTTPClient http;  //Declare an object of class HTTPClient
 
http.begin("http://192.168.178.52:3000/AGRV-728");  //Specify request destination
int httpCode = http.GET();                                                                  //Send the request
Serial.println("GET");
 
if (httpCode > 0) { //Check the returning code
 
String payload = http.getString();   //Get the request response payload
serializeJson(payload, data);
Serial.println(data["fields"]);                     //Print the response payload
 
}
 
http.end();   //Close connection
 
}
 
delay(30000);    //Send a request every 30 seconds
 
}

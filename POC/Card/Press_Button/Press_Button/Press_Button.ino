#include "ESP8266HTTPClient.h"
#include "ESP8266WiFi.h"

const char* ssid = "kernarea.de/BYOD";
const char* password =  "4m0b!l35";
int Led = 2; //LED_BUILTIN = GPIO2
int Taster = 12; //D6 = GPIO12
int value;

void setup ()
{
  Serial.begin(115200);
  pinMode (Taster, INPUT);
  WiFi.begin(ssid, password); 
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
}

void loop ()
{
  value=digitalRead(Taster);
  if(digitalRead(Taster) == 0){
    if(WiFi.status()== WL_CONNECTED){
 
      HTTPClient http;   
 
       http.begin("http://jsonplaceholder.typicode.com/posts/1"); //TODO: Right URL
       http.addHeader("Content-Type", "text/plain");            
     
       int httpResponseCode = http.PUT("PUT sent");   
     
       if(httpResponseCode>0){
     
        String response = http.getString();   
        //TODO: Change the Status on the display
        Serial.println(httpResponseCode);
        Serial.println(response);          
 
   }else{
 
    Serial.print("Error on sending PUT Request: ");
    Serial.println(httpResponseCode);
 
   }
 
   http.end();
 
 }else{
    Serial.println("Error in WiFi connection");
 }
  } 

}

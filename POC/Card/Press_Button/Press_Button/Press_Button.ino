#include "ESP8266HTTPClient.h"
#include "ESP8266WiFi.h"

const char* ssid = "kernarea.de/BYOD";
const char* password =  "4m0b!l35";
const char* abbreviation = "";
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

      http.begin("http://jiracardserver.herokuapp.com/dailyStatus");
      http.addHeader("Content-Type", "text/plain");
     
      int httpResponseCode = http.GET();

      if(httpResponseCode>0){
     
        String response = http.getString();   
        if(response == "null"){
          Serial.println("Logged in user is " + response);
          http.begin("http://jiracardserver.herokuapp.com/card/issue/1234"); //TODO: Right URL
          http.addHeader("Content-Type", "text/plain");
     
          int httpResponseCode = http.GET();
     
          if(httpResponseCode>0){
     
             String response = http.getString();   
            //TODO: Change the Story on the display
            Serial.println("Story written on display");       
 
          }else{
 
            Serial.print("Error on sending PUT Request: ");
            Serial.println(httpResponseCode);
          } 
                     
        } else {
          if(abbreviation != ""){
            char url[40];
            strcpy(url, "http://jiracardserver.herokuapp.com/status/");
            strcat(url, abbreviation);
            http.begin(url);
            int httpResponseCode = http.PUT("PUT Status sent");
            if(httpResponseCode > 0){
                //TODO: Change Status on display
                Serial.println("Status on display changed");
              } else{
                
              }
          } else{
            Serial.println("Somebody is logged in, but no Story here to change Status");
          }
        }
 
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

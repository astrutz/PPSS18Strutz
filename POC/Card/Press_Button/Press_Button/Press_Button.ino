int Led = 2; //LED_BUILTIN = GPIO2
int Taster = 12; //D6 = GPIO12
int value;

void setup ()
{
  Serial.begin(115200);
  pinMode (Taster, INPUT);
  Serial.println("init");
}

void loop ()
{
  value=digitalRead(Taster);
  if(digitalRead(Taster) == 0){
    //TODO: PUT zum Server mit neuem Status und Get für die Karte
  } 

}

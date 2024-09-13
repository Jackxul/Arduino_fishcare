#include <OneWire.h> 
#include <ESP8266WiFi.h>
#include <DallasTemperature.h> 
#define DQ_Pin 2  

OneWire oneWire(DQ_Pin);
DallasTemperature sensors(&oneWire);

void setup(void)
{
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  Serial.begin(9600);

  pinMode(7,OUTPUT);
  pinMode(4,OUTPUT); //風扇
  sensors.begin();

  WiFi.mode(WIFI_STA);

  // Disconnect from an AP if it was previously connected
  WiFi.disconnect();
  delay(100);
}

void loop(void)
{
  Serial.print("Temperatures --> ");
  sensors.requestTemperatures();
  Serial.println(sensors.getTempCByIndex(0));

  delay(2000);
  if(sensors.getTempCByIndex(0)>28){
    digitalWrite(4,HIGH);
    digitalWrite(7,HIGH);
  }else{
    digitalWrite(7,LOW);
    delay(10000);
    digitalWrite(4,LOW);
  }
  delay(2000);
}

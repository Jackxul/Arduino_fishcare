#include <OneWire.h> 
#include <ESP8266WiFi.h>
#include <DallasTemperature.h> 
#include <AsyncMqttClient.h>
#include <Ticker.h>	//Ticker Library for multi-tasking
#include <EEPROM.h>	//EEPROM Library for storing data in RTC memory
#include "pswd.h"	//Password file for WiFi and MQTT

//#include <esp_sleep.h> this is for esp32 module not for esp8266
/*
*esp_sleep_enable_timer_wakeup(SleepTimer * 1000000);
*esp_sleep_enable_ext0_wakeup(BTN_Pin, HIGH);
*esp_deep_sleep_start();
**/

#define DQ_Pin 2  // DS18B20 Data Pin(D2)
#define BTN_Pin 0 // Button Pin(D0)
#define Normal_Temp 26 // Normal Temperature
#define Cool_Temp 18 // Cool Temperature
#define Freezing_Temp 12 // Freezing Temperature
#define SleepTimer 60e6 //Sleep Timer in seconds
#define MQTT_PUBLISH_INTERVAL 300 //MQTT Publish Interval in 5 minutes

const char *mqttServer = "your.mqtt.server"; //MQTT Server
const char *topic = "temperature"; //MQTT Topic

OneWire oneWire(DQ_Pin);
DallasTemperature sensors(&oneWire);

AsyncMqttClient mqttClient; //MQTT Client object
Ticker mqttReconnectTimer; //Ticker object for MQTT reconnect
Ticker wifiReconnectTimer; //Ticker object for WiFi reconnect
Ticker Tempticker; //Ticker object for temperature check
Ticker MQTTticker; //Ticker object for MQTT publish

float TempC = Normal_Temp; //Temperature in Celsius (stored in RTC memory)

void writeToEEPROM(int address, float value) {
  EEPROM.put(address, value); // use put() to store float data
  EEPROM.commit(); // commit the data to EEPROM
}
void readFromEEPROM(int address, float &value) {
  EEPROM.get(address, value); // use get() to retrieve float data 
}
void Tempcheck(){
	sensors.requestTemperatures();
	TempC = sensors.getTempCByIndex(0);
	writeToEEPROM(0, TempC);
	Serial.print("Temperature: ");
	Serial.println(TempC);

	if(TempC >= Normal_Temp){
		//If temperature is higher than normal
		//Turn on the Fan
		//Turn on the Cooler
		digitalWrite(4, HIGH);
		digitalWrite(7, HIGH);
		//send notification
	}else if(TempC < Normal_Temp && TempC >= Cool_Temp){
		//If temperature is lower than normal
		digitalWrite(4, HIGH);
		digitalWrite(7, HIGH);

	}else if(TempC < Cool_Temp && TempC >= Freezing_Temp){	
		//If temperature is lower than cool temperature
		//Turn off the Fan
		//Turn on the Cooler
		digitalWrite(4, LOW);
		digitalWrite(7, LOW);
		Serial.println("Go to Power Saving Mode");
		//Do one MQTTpublish before going to sleep
		ESP.deepSleep(SleepTimer);
	}else{
		//If temperature is lower than cool temperature
		//Turn off the Fan
		//Turn off the Cooler
		digitalWrite(4, LOW);
		digitalWrite(7, LOW);
		
		//send notification
	}
}
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  digitalWrite(LED_BUILTIN, HIGH);
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  digitalWrite(LED_BUILTIN, LOW);
  wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  // Subscribe to topic "led" when it connects to the broker
  uint16_t packetIdSub = mqttClient.subscribe("led", 2);
  uint16_t packetIdSub2 = mqttClient.subscribe("popen", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);

// Publish on the "test" topic with qos 0
  //mqttClient.publish("test", 0, true, "test 1");
  //Serial.println("Publishing at QoS 0");
// Publish on the "test" topic with qos 1
  //uint16_t packetIdPub1 = mqttClient.publish("test", 1, true, "test 2");
  //Serial.print("Publishing at QoS 1, packetId: ");
  //Serial.println(packetIdPub1);
// Publish on the "test" topic with qos 2
  //uint16_t packetIdPub2 = mqttClient.publish("test", 2, true, "test 3");
  //Serial.print("Publishing at QoS 2, packetId: ");
  //Serial.println(packetIdPub2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  // Do whatever you want when you receive a message

  // Save the message in a variable
  String receivedMessage;
  for (int i = 0; i < len; i++) {
    Serial.println((char)payload[i]);
    receivedMessage += (char)payload[i];
  }
  // Turn the LED on or off accordingly to the message content
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

void onMqttPublish(){
  mqttClient.publish(topic, 1, true, String(sensors.getTempCByIndex(0)).c_str());
}
void IRAM_ATTR handleButtonPress() {
    // interrupt handler
    Serial.println("Button pressed! Waking up...");
    // do something here but keep it short
}
void setup(void)
{
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  EEPROM.begin(128);
  readFromEEPROM(0, TempC);
  pinMode(BTN_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_Pin), handleButtonPress, FALLING);

  pinMode(7,OUTPUT);
  pinMode(4,OUTPUT); //風扇
  digitalWrite(4,HIGH); // initialize the fan as on
  digitalWrite(7,HIGH); // initialize the cooler as on
  sensors.begin();


  WiFi.mode(WIFI_STA);

  // Disconnect from an AP if it was previously connected
  WiFi.disconnect();

  connectToWifi();
  Tempticker.attach(30, Tempcheck); //Check temperature every 10 seconds
  MQTTticker.attach(MQTT_PUBLISH_INTERVAL, onMqttPublish); //Publish temperature every 5 minutes
}

void loop(void)
{
  delay(100);
}

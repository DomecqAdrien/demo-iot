#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <PubSubClient.h>

const int LED_PIN = 8;
const int TEMP_CAPTOR = 0;
const int LIGHT_CAPTOR = 1;

String mainTopic = "Captors";
String ledPowerTopic = mainTopic + "/ledPower"; 
String ledStateTopic = mainTopic + "/ledState";

Ethernet WSA; // WSAStartup
EthernetClient ethClient;
PubSubClient mqttClient;

float valueTemp = 0.0;
float valueLight = 0.0;

String payloadToString(byte *payload, unsigned int length){
	char buffer[length];
	sprintf(buffer, "%.*s", length, payload);
	return String(buffer);
}

void switchLed(bool on)
{

	if(on){
    	digitalWrite(LED_PIN, 1);
	}
  	else{
    	digitalWrite(LED_PIN, 0);
  	}
	if (!mqttClient.publish(ledStateTopic.c_str(), String(on).c_str())){
		Serial.println("Unable to publish led state value..");
	}
}

void actionCallback(char *topicChar, byte *payloadByte, unsigned int length){
	Serial.println("New message received");
	String topic = String(topicChar);
	String payload = payloadToString(payloadByte, length);

	Serial.print("Topic: ");
	Serial.println(topic);

	Serial.print("Payload: ");
	Serial.println(payload);

	if (!topic.equals(ledPowerTopic)){
		return;
	}

	if(payload.equalsIgnoreCase("on")){
    	switchLed(true);
		}
  	else if(payload.equalsIgnoreCase("off")){
    	switchLed(false);
  	}
}

void setupMqttClient(){
	mqttClient.setClient(ethClient);
	mqttClient.setServer("192.168.1.24", 1883);//mqtt.flespi.io
	mqttClient.setCallback(actionCallback);
}

void setup(){
	Serial.begin(9600);
	pinMode(LED_PIN, OUTPUT);
	setupMqttClient();

	analogWrite(TEMP_CAPTOR, 10 / 5.0 * 1024.0);
	analogWrite(LIGHT_CAPTOR, 250);
}

void subscribeActionTopic(){
	if (!mqttClient.subscribe(ledPowerTopic.c_str()))
	{
		Serial.println("Topic led action subscribe error");
		return;
	}
	Serial.println("Topic led action subscribe success");
}

void connectMqtt(){
	if (!mqttClient.connected())
	{
		Serial.println("Trying to connect to mqtt broker");
		if (!mqttClient.connect("Test","SJuXSGwx8qdXdyNkxkHBd7ydHBxOYlqk",""))//, "Lg45hq1UnetotXiG5ow3ZVLM3vWnaCKJIFFNTux6yOiPBqmuEInrtX1kiOygg6QO", ""
		{
			Serial.println("Mqtt broker connection failed");
			return;
		}
		Serial.println("Mqtt broker connection success");
		subscribeActionTopic();
	}
}

void publishCaptor(int pin, float value){
	if (!mqttClient.connected())
	{
		Serial.println("Unable to publish captor value since mqtt broker insn't connect");
		return;
	}
	String finalTopic = pin == LIGHT_CAPTOR ? mainTopic + "/Light" : mainTopic + "/Temp";
	Serial.println(String(pin) + " / " +finalTopic);
	if (!mqttClient.publish(finalTopic.c_str(), String(value).c_str()))
	{
		Serial.println("Unable to publish captor value..");
	}
}

float getTemp(int pin){
	float analogValue = analogRead(pin);
	float temp = analogValue * 5.0 / 1024.0;
	Serial.print("temp value : ");
	Serial.println(temp);
	return temp;
}

float getLight(int pin){
	float light = analogRead(pin);
	Serial.print("light value : ");
	Serial.println(light);
	return light;
}

void loop(){
	connectMqtt();
	publishCaptor(LIGHT_CAPTOR, getLight(LIGHT_CAPTOR));
	publishCaptor(TEMP_CAPTOR, getTemp(TEMP_CAPTOR));
	mqttClient.loop();
	delay(1000);
}

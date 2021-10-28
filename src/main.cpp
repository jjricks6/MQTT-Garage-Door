//MQTT Garage Door Sensor

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define SWITCH D4

// ------- Connection Info for WIFI -------
const char* ssid = "mywifi";
const char* password = "mypass";
const char* server = "192.168.1.111";    // IP Address of the MQTT Broker
char* dataTopic = "/switch";
char* connectTopic = "/example/connect";
char* disconnectTopic = "/example/disconnect";
char* user = "mymqttuser";
char* pass = "mypass";

// ------- Hostname of this Arduino -------
String macAddr = WiFi.macAddress();
String host = "arduino-" + macAddr.substring(15) ;  // Set a client ID for this device (should not match other MQTT devices)

// ------- Global Variables and Classes -------
String message;
long timer;
WiFiClient wifiClient;                  // Instantiate wifi
PubSubClient mqttClient(wifiClient);    // Instantiate mqtt client

// Start switch in disconnected state
bool switch_state = 1;
bool prev_state = 0;


// Callback function to read messages for subscribed topics
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  Serial.println("\nMQTT Garage Door Sensor");
  Serial.print("Connecting to '");
  Serial.print(ssid);
  Serial.println("' network");

  // Connect to Wifi
  WiFi.hostname(host);
  WiFi.begin(ssid, password);

  //Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  delay (1000);

  mqttClient.setServer(server, 1883);   // Set up MQTT Connection Info
  mqttClient.setCallback(callback);     // Set callback action for receiving messages

  Serial.println("Connecting to MQTT Broker");
  char hostChar[host.length()+1];
  host.toCharArray(hostChar, host.length()+1 );
  
  // This is the simple way to connect
  if (mqttClient.connect(hostChar, user, pass)) { 
    Serial.println("MQTT Connected");
    mqttClient.publish(connectTopic, hostChar);
    Serial.println(mqttClient.state());

  } else {
    Serial.println("MQTT Connection Failure");
    Serial.println(mqttClient.state());
  }

  // ------- MQTT Subscribe to a topic -------
  mqttClient.subscribe(dataTopic);

  pinMode(SWITCH, INPUT_PULLUP);
}

void loop() {
  // Loop to check for new messages
  mqttClient.loop();

  // Read the switch state
  switch_state = digitalRead(SWITCH);

  // Switch just disconnected
  if((prev_state == 1) && (switch_state == 1)){
    prev_state = 0;

    message = "off";
    char messageChar[message.length() + 1];
    message.toCharArray(messageChar, message.length() + 1);
    mqttClient.publish(dataTopic, messageChar);  // Send message to MQTT client
  }

  // Switch just connected
  if((prev_state == 0) && (switch_state == 0)){
    prev_state = 1;

    message = "on";
    char messageChar[message.length() + 1];
    message.toCharArray(messageChar, message.length() + 1);
    mqttClient.publish(dataTopic, messageChar); // Send message to MQTT client
  }
}
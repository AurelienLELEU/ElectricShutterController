#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "Wifi SSID";
const char* password = "Wifi password";

// MQTT broker details
const char* mqtt_server = "MQTT broker IP";
const int mqtt_port = 1883;
const char* mqtt_topic = "MQTT topic";

// Pin numbers for the relays
const int relayDown = 4; // Pin to control the first relay (relayDown)
const int relayUp = 5; // Pin to control the second relay (relayUp)

// Name of the ESP32
const char* esp32Name = "Esp32 Name";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Setup serial communication
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    if (client.connect(esp32Name)) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Failed MQTT connection, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }

  // Set relay pins as OUTPUT
  pinMode(relayDown, OUTPUT);
  pinMode(relayUp, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// MQTT message callback function
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Received MQTT message: ");
  Serial.println(message);

  // Check if the message contains the ESP32's name + command
  if (message.startsWith(esp32Name)) {
    // Extract the command from the message
    String command = message.substring(strlen(esp32Name) + 1);
    
    // Control the relays based on the received command
    if (command == "down") {
      digitalWrite(relayDown, HIGH);  // Turn on relayDown
      digitalWrite(relayUp, LOW);   // Turn off relayUp
      Serial.println("Shutter moving down");
    } else if (command == "up") {
      digitalWrite(relayDown, LOW);   // Turn off relayDown
      digitalWrite(relayUp, HIGH);  // Turn on relayUp
      Serial.println("Shutter moving up");
    } else if (command == "stop") {
      digitalWrite(relayDown, LOW);   // Turn off relayDown
      digitalWrite(relayUp, LOW);   // Turn off relayUp
      Serial.println("Shutter stopped");
    }
  }
}

// Reconnect to the MQTT broker
void reconnect() {
  while (!client.connected()) {
    if (client.connect(esp32Name)) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Failed MQTT connection, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

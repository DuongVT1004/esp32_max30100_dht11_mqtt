#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wifi settings
const char *ssid = "Galaxy S20 FE 03E4";
const char *password = "byic7473";

const char * mqtt_broker = "test.mosquitto.org";
const char *mqtt_topic = "/Weather/Hum_Tem";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void connectToWiFi();

void connectToMQTTBroker();

void mqttCallback(char *topic, byte *payload, unsigned int length);

void setup()
{
  Serial.begin(115200);
  connectToWiFi();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(mqttCallback);
  connectToMQTTBroker();
}

void connectToWiFi()
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to the WiFi network");
}

void connectToMQTTBroker()
{
  while(!mqtt_client.connected())
  {
    String client_id = "esp8266-client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
    if(mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) 
    {
      Serial.println("Connected to MQTT Broker");
      mqtt_client.subscribe(mqtt_topic);
      mqtt_client.publish(mqtt_topic, "I am Duong");
    }
    else
    {
      Serial.print("Failed to connect to MQTT Broker, rc = ");
      Serial.print(mqtt_client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String message;
  for(int i = 0; i < length; i++)
  {
    message += (char) payload[i];
  }
  Serial.print(message);
  Serial.print("\n");
}

void loop()
{
  if(!mqtt_client.connected())
  {
    connectToMQTTBroker();
  }
  mqtt_client.loop();
}

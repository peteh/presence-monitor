#include <Arduino.h>
#include <SoftwareSerial.h>
#include <esplog.h>
#include <MqttDevice.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
#include <mdns.h>
#endif


#include "config.h"
#include "Sensor.h"
#include "utils.h"

Sensor presenceSensor(D2, D1);
MqttDevice mqttDevice(composeClientID().c_str(), "Presence", "Presence Sensor HLK-LD1115H", "maker_pt");
MqttBinarySensor mqttOccupancy(&mqttDevice, "occupancy", "Occupancy");
MqttBinarySensor mqttMotion(&mqttDevice, "motion", "Motion");
WiFiClient net;
PubSubClient client(net);

const char *HOMEASSISTANT_STATUS_TOPIC = "homeassistant/status";
const char *HOMEASSISTANT_STATUS_TOPIC_ALT = "ha/status";

bool g_motion = false;
bool g_occupancy = false;

unsigned long lastCommandSend = millis();

void publishConfig(MqttEntity *entity)
{
    String payload = entity->getHomeAssistantConfigPayload();
    char topic[255];
    entity->getHomeAssistantConfigTopic(topic, sizeof(topic));
    client.publish(topic, payload.c_str());

    entity->getHomeAssistantConfigTopicAlt(topic, sizeof(topic));
    client.publish(topic,
                   payload.c_str());
}

void publishConfig()
{
    publishConfig(&mqttOccupancy);
    publishConfig(&mqttMotion);
}

void connectToMqtt()
{
    log_info("Connecting to MQTT...");
    // TODO: add security settings back to mqtt
    // while (!client.connect(mqtt_client, mqtt_user, mqtt_pass))
    while (!client.connect(composeClientID().c_str()))
    {
        log_debug(".");
        delay(4000);
    }

    client.subscribe(HOMEASSISTANT_STATUS_TOPIC);
    client.subscribe(HOMEASSISTANT_STATUS_TOPIC_ALT);

    // TODO: solve this somehow with auto discovery lib
    // client.publish(mqttTopic(MQTT_TOPIC_NONE, MQTT_ACTION_NONE).c_str(), "online");
    publishConfig();
}

void connectToWifi()
{
    log_info("Connecting to wifi...");
    // TODO: really forever? What if we want to go back to autoconnect?
    while (WiFi.status() != WL_CONNECTED)
    {
        log_debug(".");
        delay(1000);
    }
    log_info("Wifi connected!");
}

void publishState()
{
  char buffer[255];
  snprintf(buffer, sizeof(buffer), "{\"occupancy\": \"%s\", \"motion\": \"%s\"}",
           g_occupancy ? mqttOccupancy.getOnState() : mqttOccupancy.getOffState(),
           g_motion ? mqttMotion.getOnState() : mqttMotion.getOffState());

  client.publish(mqttOccupancy.getStateTopic(), buffer);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    log_info("Message arrived [%s]", topic);
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // publish config when homeassistant comes online and needs the configuration again
    if (strcmp(topic, HOMEASSISTANT_STATUS_TOPIC) == 0 ||
             strcmp(topic, HOMEASSISTANT_STATUS_TOPIC_ALT) == 0)
    {
        if (strncmp((char *)payload, "online", length) == 0)
        {
            publishConfig();
        }
    }
}

void setup()
{
  mqttOccupancy.setValueTemplate("{{value_json.occupancy}}");
  mqttMotion.setCustomStateTopic(mqttOccupancy.getStateTopic());
  mqttMotion.setValueTemplate("{{value_json.motion}}");

  Serial.begin(115200);
  presenceSensor.begin();
  presenceSensor.setMotionThreshold(200);
  presenceSensor.setOccupancyThreshold(200);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(composeClientID().c_str());
  WiFi.setAutoConnect(true);
  WiFi.begin(wifi_ssid, wifi_pass);

  connectToWifi();
  log_info("Connected to SSID: %s", wifi_ssid);
  log_info("IP address: %s", WiFi.localIP());

  client.setBufferSize(512);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("Boot Complete");
}


void loop()
{
  if (WiFi.status() != WL_CONNECTED)
    {
        connectToWifi();
    }
    if (!client.connected())
    {
        connectToMqtt();
    }

  if (millis() - lastCommandSend > 10000 && false)
  {
    Serial.println("Sending Command");
    lastCommandSend = millis();
    Serial.println("Finished Command");
  }
  client.loop();
  presenceSensor.loop();
  bool occupancy = presenceSensor.hasOccupancy();
  bool motion = presenceSensor.hasMotion();
  bool stateChanged = false;
  if (g_occupancy != occupancy)
  {
    stateChanged = true;
    g_occupancy = occupancy;
    log_info("Occupancy: %d", occupancy);
  }
  if (g_motion != motion)
  {
    stateChanged = true;
    g_motion = motion;
    log_info("Motion: %d", motion);
  }
  if(stateChanged)
  {
    publishState();
  }
}
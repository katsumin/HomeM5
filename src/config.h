#include <Arduino.h>

// WiFi
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASS "WIFI_PASS"

// NTP
#define NTP_SERVER "time.nist.gov"

// Ethernet
byte mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// MQTT
#define MQTT_BROKER_HOST "MQTT BROKER ADDRESS"
#define MQTT_BROKER_PORT 1883
#define MQTT_BROKER_USER "MQTT BROKER USER"
#define MQTT_BROKER_PASS "MQTT BROKER PASSWORD"
#define MQTT_PUBLISH_BUFFER 1024
#define MQTT_PUBLISH_TOPIC "home_measure"

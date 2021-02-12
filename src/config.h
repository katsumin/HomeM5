#include <Arduino.h>

// smartmeter
#define PWD "pwd"
#define BID "bid"

// WiFi
#define WIFI_SSID "ssid"
#define WIFI_PASS "pass"

// InfluxDB
// #define INFLUX_SERVER "192.168.1.121"
#define INFLUX_SERVER "influx"
#define INFLUX_DB "smartmeter"

// LAN
#define ECOCUTE_ADDRESS "0.0.0.0"
#define AIRCON_ADDRESS "0.0.0.0"
#define SMARTMETER_ADDRESS "0.0.0.0"

// NTP
#define NTP_SERVER "time.nist.gov"

// Ethernet
byte mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

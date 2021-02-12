#include <queue>
#include <M5Stack.h>
#include <time.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Ethernet.h>
#include <Wire.h>
// #include <BME280_t.h> // import BME280 template library
#include <Adafruit_BME280.h>
#include "InfluxDb.h"
#include "smartmeter.h"
#include "config.h"
#include "debugView.h"
#include "FunctionButton.h"
#include "MainView.h"
#include "HeadView.h"
#include "PowerView.h"
#include "Rtc.h"
#include "EchonetUdp.h"
#include "Ecocute.h"
#include "Aircon.h"
#include "log.h"
#include "DataStore.h"
#include "View.h"
#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 26
// #define TEST

// instances
FunctionButton btnB(&M5.BtnB);
FunctionButton btnC(&M5.BtnC);
DebugView debugView(0, 100, 320, SCROLL_SIZE * 10);
InfluxDb influx(INFLUX_SERVER, INFLUX_DB);
// SmartMeter sm;
NtpClient ntp;
Rtc rtc;
DataStore dataStore;
MainView mainView(&dataStore);
PowerView powerView(&dataStore, &rtc);
ViewController viewController;

#define TIMEZONE 9 * 3600

boolean bEco = false;
void ecocuteCallback(Ecocute *ec)
{
    // sd_log.out("Ecocute callback");
    Serial.println("Ecocute callback");
    bEco = true;
}

boolean bAir = false;
void airconCallback(Aircon *ac)
{
    // sd_log.out("Aircon callback");
    Serial.println("Aircon callback");
    bAir = true;
}
boolean bSma = false;
void smartmeterCallback(SmartMeter *sm)
{
    Serial.println("Smartmeter callback");
    bSma = true;
}

void callbackNtp(void *arg)
{
    Serial.println("callbacked !");
    btnB.enable("NTP");
}

WiFiMulti wm;
void nw_init()
{
    Serial.println();
    SPI.begin(SCK, MISO, MOSI, -1);
    Ethernet.init(CS); // Ethernet/Ethernet2
    IPAddress addr;
    char buf[64];
    UDP *udpNtp = nullptr;
    UDP *udpEchonet = nullptr;
    if (Ethernet.linkStatus() == LinkON) // Ethernet
    {
        // Ethernet
        Serial.println("Ethernet connected");
        Serial.println("IP address: ");
        Ethernet.begin(mac);
        addr = Ethernet.localIP();
        influx.setEthernet(true);
        echonetUdp.setEthernet(true);
        snprintf(buf, sizeof(buf), "%s(Ethernet)", addr.toString().c_str());
        headView.setNwType("Ethernet");
        udpNtp = new EthernetUDP();
        udpEchonet = new EthernetUDP();
    }
    else
    {
        // WiFi
        Serial.print("Connecting to WIFI");
        wm.addAP(WIFI_SSID, WIFI_PASS);
        while (wm.run() != WL_CONNECTED)
        {
            Serial.print(".");
            delay(100);
        }
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        addr = WiFi.localIP();
        influx.setEthernet(false);
        echonetUdp.setEthernet(false);
        snprintf(buf, sizeof(buf), "%s(WiFi)", addr.toString().c_str());
        headView.setNwType("WiFi");
        udpNtp = new WiFiUDP();
        udpEchonet = new WiFiUDP();
    }
    Serial.println(buf);
    headView.setIpAddress(addr);

    // RTC
    ntp.init(udpNtp, (char *)NTP_SERVER, random(10000, 19999));
    ntp.setCallback(callbackNtp, nullptr);
    rtc.init(&ntp);
    // echonet
    echonetUdp.init(udpEchonet);
    // echonetUdp.getServer();
    delay(1000);
    // ecocute
    ecocute.init(&echonetUdp, ECOCUTE_ADDRESS, ecocuteCallback);
    // aircon
    aircon.init(&echonetUdp, AIRCON_ADDRESS, airconCallback);
    // smartmeter
    smartmeter.init(&echonetUdp, SMARTMETER_ADDRESS, smartmeterCallback);
    smartmeter.setDataStore(&dataStore);
}

Adafruit_BME280 bme; // I2C
// BME280<> BMESensor; // instantiate sensor
void updateBme(boolean withInflux)
{
    while (true)
    {
        // // BMESensor.refresh(); // read current sensor data
        // float p = BMESensor.pressure;
        float p = bme.readPressure();
        if (p >= 90000.0)
            break;
        Serial.println(p);
        delay(100);
    }
    // float temp = BMESensor.temperature;
    // float hum = BMESensor.humidity;
    // float press = BMESensor.pressure / 100.0F;
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float press = bme.readPressure() / 100.0F;
    dataStore.setTemperature(temp);
    dataStore.setHumidity(hum);
    dataStore.setPressure(press);
    if (withInflux)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "room temp=%.1f,hum=%.1f,press=%.1f", temp, hum, press);
        debugView.output(buf);
        int res = influx.write(buf);
        debugView.output(res);
    }
}

#define VIEWKEY_MAIN ((const char *)"MAIN")
#define VIEWKEY_POWER ((const char *)"POWER")
// CONNECT_STATE preState;
void setup()
{
    M5.begin();

    // DataStore
    dataStore.init();

    //  View
    M5.Lcd.clear();
    headView.init();
    headView.setRtc(&rtc);
    sd_log.setRtc(&rtc);
    viewController.setView(VIEWKEY_MAIN, &mainView, VIEWKEY_POWER);
    viewController.setView(VIEWKEY_POWER, &powerView, VIEWKEY_MAIN);
    viewController.changeNext();

    // // スマートメータ
    btnC.enable(viewController.getNextKey());

    // LAN
    btnB.disable("NTP");
    nw_init();

    // Sensor
    Wire.begin(GPIO_NUM_21, GPIO_NUM_22); // initialize I2C that connects to sensor
#ifndef TEST
    // BMESensor.begin(); // initalize bme280 sensor
    if (!bme.begin(0x76))
    {
        Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
        while (1)
            delay(10);
    }
    updateBme(true);
#endif
    // delay(1000);
    ecocute.request();
    aircon.request();
    smartmeter.request();
}

time_t rtcTest = 0;
float wattHourTest = 0.0;
unsigned long preMeas = millis();
unsigned long preView = preMeas;
#define INTERVAL (30 * 1000)
#define VIEW_INTERVAL (1000)
void loop()
{
    long cur = millis();
    long d = cur - preMeas;
    if (d < 0)
        d += ULONG_MAX;
    if (d >= INTERVAL) // debug
    {
        preMeas = cur;

        char buf[64];
        snprintf(buf, sizeof(buf), "%ld -> %ld, memory(%d)", preMeas, d, xPortGetFreeHeapSize());
        debugView.output(buf);

        // BME280
#ifndef TEST
        updateBme(true);
#endif

        // smartmeter
        smartmeter.request();
        // ecocute
        ecocute.request();
        // aircon
        aircon.request();
    }

    char buf[64];
    if (bEco)
    {
        dataStore.setEcoPower(ecocute.getPower());
        dataStore.setEcoTank(ecocute.getTank());
        snprintf(buf, sizeof(buf), "ecocute power=%ld,powerSum=%ld,tank=%ld", ecocute.getPower(), ecocute.getTotalPower(), ecocute.getTank());
        debugView.output(buf);
        int res = influx.write(buf);
        debugView.output(res);
        bEco = false;
    }
    if (bAir)
    {
        dataStore.setAirPower(aircon.getPower());
        dataStore.setAirTempOut(aircon.getTempOut());
        dataStore.setAirTempRoom(aircon.getTempRoom());
        snprintf(buf, sizeof(buf), "aircon power=%ld,tempOut=%ld,tempRoom=%ld", aircon.getPower(), aircon.getTempOut(), aircon.getTempRoom());
        debugView.output(buf);
        int res = influx.write(buf);
        debugView.output(res);
        bAir = false;
    }

    d = cur - preView;
    if (d < 0)
        d += ULONG_MAX;
    if (d >= VIEW_INTERVAL)
    {
        preView = cur;
        headView.update();
        viewController.update();
    }
    if (btnB.isEnable() && btnB.getButton()->wasPressed())
    {
        rtc.adjust();
        btnB.disable("NTP");
    }
    else if (btnC.isEnable() && btnC.getButton()->wasPressed())
    {
        viewController.changeNext();
        btnC.enable(viewController.getNextKey());
    }
    delay(1);
    M5.update();
}
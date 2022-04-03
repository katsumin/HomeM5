#include "View.h"
#define _M5DISPLAY_H_
class M5Display
{
};
#include <M5Stack.h>
#include <time.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Ethernet3.h>
#include <NTPClient.h>
#include "InfluxDb.h"
#include "config.h"
#include "FunctionButton.h"
#include "MainView.h"
#include "HeadView.h"
#include "DataStore.h"
#include "EthernetManager.h"

#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 26
#define RST 5
#define TEST

// instances
static TFT_eSPI lcd;
FunctionButton btnB(&M5.BtnB, &lcd, POS_B_X);
FunctionButton btnC(&M5.BtnC, &lcd, POS_C_X);
ViewController viewController(&btnC, &lcd);
HeadView headView(&lcd);
DataStore dataStore(&viewController);
MainView mainView(&dataStore, &lcd);

const long gmtOffset_sec = 9 * 3600; // 9時間の時差を入れる

EthernetManager *em;
NTPClient *ntp;
void nw_init()
{
    Serial.println();

    SPI.begin(SCK, MISO, MOSI, -1);
    IPAddress addr;
    // char buf[64];
    UDP *udpNtp = nullptr;
    UDP *udpUni = nullptr;
    UDP *udpMulti = nullptr;

    headView.init();
    Ethernet.setCsPin(CS);   // Ethernet3
    Ethernet.setRstPin(RST); // Ethernet3
    boolean isEther = false;
    Serial.print("Connecting to Ethernet");
    Ethernet.hardreset();
    if (Ethernet.begin(mac) != 0)
    {
        // Ethernet
        Serial.println("Ethernet connected");
        Serial.println("IP address: ");
        addr = Ethernet.localIP();
        isEther = true;
        headView.setNwType("Ethernet");
        udpNtp = new EthernetUDP();
        udpUni = new EthernetUDP();
        udpMulti = new EthernetUDP();
        dataStore.init(new EthernetClient(), INFLUX_SERVER, INFLUX_DB);
    }
    else
    {
        // WiFi
        Serial.print("Connecting to WIFI");
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.print(".");
            delay(100);
        }
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        addr = WiFi.localIP();
        headView.setNwType("WiFi");
        udpNtp = new WiFiUDP();
        udpUni = nullptr;
        udpMulti = new WiFiUDP();
        dataStore.init(new WiFiClient(), INFLUX_SERVER, INFLUX_DB);
    }
    headView.setIpAddress(addr);
    em = new EthernetManager(udpMulti, udpUni);
    em->setDataStore(&dataStore);

    // NTP
    ntp = new NTPClient(*udpNtp, NTP_SERVER, gmtOffset_sec, 10 * 60 * 1000);
    ntp->begin();
    ntp->update();
    headView.setNtp(ntp);
}

#define INTERVAL (60)
#define INFLUX (0)
#define SCAN (30)
void static influxTask(void *arm)
{
    Serial.println("InfluxDB Task start.");
    unsigned long pre = millis();
    while (true)
    {
        delay(700); // 0.7s wait
        unsigned long epoch = ntp->getEpochTime();
        if (epoch % INTERVAL == INFLUX)
        {
            Serial.printf("%s influx start\n", ntp->getFormattedTime().c_str());
            pre = millis();
            // xSemaphoreTake(_mutex, portMAX_DELAY);
            dataStore.updateInflux(epoch - gmtOffset_sec);
            // xSemaphoreGive(_mutex);
            unsigned long duration = millis() - pre;
            Serial.printf("%s influx duration: %d\n", ntp->getFormattedTime().c_str(), duration);
            // Serial.println();
        }
        else if (epoch % INTERVAL == SCAN)
        {
            Serial.printf("%s scan start\n", ntp->getFormattedTime().c_str());
            pre = millis();
            // xSemaphoreTake(_mutex, portMAX_DELAY);
            em->request();
            // xSemaphoreGive(_mutex);
            unsigned long duration = millis() - pre;
            Serial.printf("%s scan duration: %d\n", ntp->getFormattedTime().c_str(), duration);
            // Serial.println();
        }
    }
    Serial.println("InfluxDB Task end.");
    vTaskDelete(NULL);
}

#define VIEWKEY_MAIN ((const char *)"MAIN")
#define VIEWKEY_POWER ((const char *)"POWER")
void setup()
{
    lcd.init();
    M5.begin();

    //  View
    viewController.setView(VIEWKEY_MAIN, &mainView);

    // LAN
    btnB.disable("NTP");
    nw_init();
    ntp->update();
    btnB.enable("NTP");

    em->scan();

    xTaskCreate(influxTask, "InfluxTask", 4096, NULL, 5, NULL);

    // set mutex
    xSemaphoreHandle mutex = xSemaphoreCreateMutex();
    em->setMutex(mutex);

    // enable WDT
    enableCore0WDT(); // core 0 のWDT有効
    enableCore1WDT(); // core 1 のWDT有効
}

unsigned long preEpoch = 0;
boolean db = false;
void loop()
{
    em->update();

    unsigned long epoch = ntp->getEpochTime();
    if (preEpoch != epoch)
    {
        preEpoch = epoch;
        headView.update();
        viewController.update();
    }

    if (btnB.isEnable() && btnB.getButton()->wasPressed())
    {
        btnB.disable("NTP");
        ntp->update();
        btnB.enable("NTP");
    }
    else if (btnC.isEnable() && btnC.getButton()->wasPressed())
    {
        viewController.changeNext();
    }
    delay(1);
    M5.update();
}
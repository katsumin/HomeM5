#include "View.h"
#define _M5DISPLAY_H_
class M5Display
{
};
#include <M5Stack.h>
// #include <queue>
#include <time.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Ethernet3.h>
#include <NTPClient.h>
#include "InfluxDb.h"
#include "config.h"
// #include "debugView.h"
#include "FunctionButton.h"
#include "MainView.h"
#include "HeadView.h"
// #include "PowerView.h"
#include "DataStore.h"
#include "EthernetManager.h"

#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 26
#define RST 5
#define TEST

// instances
// TFT_eSPI *lcd = &M5.Lcd;
static TFT_eSPI lcd;
// FunctionButton btnA(&M5.BtnA, &lcd, POS_A_X);
FunctionButton btnB(&M5.BtnB, &lcd, POS_B_X);
FunctionButton btnC(&M5.BtnC, &lcd, POS_C_X);
// DebugView debugView(0, 100, 320, SCROLL_SIZE * 10);
// InfluxDb influx(INFLUX_SERVER, INFLUX_DB);
// SmartMeter sm;
// NtpClient ntp;
// Rtc rtc;
// PowerView *powerView;
ViewController viewController(&btnC, &lcd);
HeadView headView(&lcd);
DataStore dataStore(&viewController);
MainView mainView(&dataStore, &lcd);

const long gmtOffset_sec = 9 * 3600; //9時間の時差を入れる

EthernetManager *em;
NTPClient *ntp;
// WiFiMulti wm;
void nw_init()
{
    Serial.println();

    SPI.begin(SCK, MISO, MOSI, -1);
    // Ethernet.init(CS); // Ethernet/Ethernet2
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
    // int ret = Ethernet.begin(mac);
    // for (int i = 0; i < 10; i++)
    // {
    //     Serial.print(".");
    //     ret = Ethernet.begin(mac);
    //     if (ret != 0)
    //         break;
    //     delay(100);
    // }
    if (Ethernet.begin(mac) != 0)
    {
        // Ethernet
        Serial.println("Ethernet connected");
        Serial.println("IP address: ");
        // Ethernet.begin(mac);
        addr = Ethernet.localIP();
        isEther = true;
        // echonetUdp.setEthernet(true);
        // snprintf(buf, sizeof(buf), "%s(Ethernet)", addr.toString().c_str());
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
        // wm.addAP(WIFI_SSID, WIFI_PASS);
        // while (WiFi.run() != WL_CONNECTED)
        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.print(".");
            delay(100);
        }
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        addr = WiFi.localIP();
        // echonetUdp.setEthernet(false);
        // snprintf(buf, sizeof(buf), "%s(WiFi)", addr.toString().c_str());
        headView.setNwType("WiFi");
        udpNtp = new WiFiUDP();
        udpUni = nullptr;
        udpMulti = new WiFiUDP();
        dataStore.init(new WiFiClient(), INFLUX_SERVER, INFLUX_DB);
    }
    // influx.setEthernet(isEther);
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
xSemaphoreHandle _mutex = xSemaphoreCreateMutex();
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
            pre = millis();
            xSemaphoreTake(_mutex, portMAX_DELAY);
            dataStore.updateInflux(epoch - gmtOffset_sec);
            xSemaphoreGive(_mutex);
            unsigned long duration = millis() - pre;
            Serial.printf("influx duration: %d", duration);
            Serial.println();
        }
        else if (epoch % INTERVAL == SCAN)
        {
            pre = millis();
            xSemaphoreTake(_mutex, portMAX_DELAY);
            em->request();
            xSemaphoreGive(_mutex);
            unsigned long duration = millis() - pre;
            Serial.printf("scan duration: %d", duration);
            Serial.println();
        }
    }
    Serial.println("InfluxDB Task end.");
    vTaskDelete(NULL);
}
// void static scanTask(void *arm)
// {
//     Serial.println("Scan Task start.");
//     while (true)
//     {
//         delay(700); // 0.7s wait
//         unsigned long epoch = ntp->getEpochTime();
//         if (epoch % INTERVAL == SCAN)
//         {
//             unsigned long pre = millis();
//             em->request();
//             unsigned long duration = millis() - pre;
//             Serial.printf("scan duration: %d", duration);
//             Serial.println();
//         }
//     }
//     Serial.println("Scan Task end.");
//     vTaskDelete(NULL);
// }

#define VIEWKEY_MAIN ((const char *)"MAIN")
#define VIEWKEY_POWER ((const char *)"POWER")
void setup()
{
    lcd.init();
    M5.begin();

    //  View
    viewController.setView(VIEWKEY_MAIN, &mainView);
    // powerView = new PowerView(&dataStore, &lcd);
    // viewController.setView(VIEWKEY_POWER, powerView);
    // viewController.changeNext();
    // btnC.enable(viewController.getNextKey());

    // LAN
    btnB.disable("NTP");
    nw_init();
    ntp->update();
    btnB.enable("NTP");

    em->scan();

    // xTaskCreate(scanTask, "ScanTask", 4096, NULL, 5, NULL);
    xTaskCreate(influxTask, "InfluxTask", 4096, NULL, 5, NULL);
}

unsigned long preEpoch = 0;
boolean db = false;
void loop()
{
    em->update();

    unsigned long epoch = ntp->getEpochTime();
    if (preEpoch != epoch)
    {
        // Serial.printf("epoch: %ld", epoch);
        // Serial.println();
        preEpoch = epoch;
        // if (epoch % INTERVAL == 0)
        // {
        //     em->request();
        // }
        headView.update();
        viewController.update();
    }

    if (btnB.isEnable() && btnB.getButton()->wasPressed())
    {
        btnB.disable("NTP");
        // Serial.println("NTP");
        ntp->update();
        btnB.enable("NTP");
    }
    else if (btnC.isEnable() && btnC.getButton()->wasPressed())
    {
        viewController.changeNext();
        // btnC.enable(viewController.getNextKey());
        // }
        // else if (btnA.isEnable() && btnA.getButton()->wasReleased())
        // {
        //     Serial.println("DB on/off");
        //     db = !db;
        //     if (db)
        //         btnA.enable("DB on");
        //     else
        //         btnA.enable("DB off");
    }
    delay(1);
    M5.update();
}
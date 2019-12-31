#include <queue>
#include <M5Stack.h>
#include <time.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Ethernet.h>
#include <Wire.h>
#include <BME280_t.h> // import BME280 template library
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
FunctionButton btnA(&M5.BtnA);
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

// /*
// スマートメータ接続タスク
// */
// void bp35c0_monitoring_task(void *arm)
// {
//     // bJoined = false;
//     Serial.println("task start");
//     // bTaskRunning = true;
//     // スマートメータ接続
//     sm.disconnect();
//     delay(500);
//     sm.connect(PWD, BID);

//     Serial.println("task stop");
//     vTaskDelete(NULL);
// }

#define TIMEZONE 9 * 3600
// boolean rcvEv29 = false;
// void bp35c0_polling()
// {
//     int res;
//     char buf[1024];
//     RCV_CODE code = sm.polling(buf, sizeof(buf));
//     switch (code)
//     {
//     case RCV_CODE::ERXUDP:
//     case RCV_CODE::OTHER:
//         // UDP受信
//         debugView.output(buf);
//         sd_log.out(buf);
//         break;
//     case RCV_CODE::ERXUDP_E7:
//         dataStore.setPower(sm.getPower());
//         snprintf(buf, sizeof(buf), "power value=%ld", sm.getPower());
//         debugView.output(buf);
//         res = influx.write(buf);
//         debugView.output(res);
//         break;
//     case RCV_CODE::ERXUDP_EAB:
//         dataStore.setWattHourPlus(sm.getWattHourPlus(), sm.getTimePlus() + TIMEZONE);
//         dataStore.setWattHourMinus(sm.getWattHourMinus(), sm.getTimeMinus() + TIMEZONE);
//         snprintf(buf, sizeof(buf), "+power value=%.1f %ld000000000", sm.getWattHourPlus(), sm.getTimePlus());
//         debugView.output(buf);
//         res = influx.write(buf);
//         debugView.output(res);
//         snprintf(buf, sizeof(buf), "-power value=%.1f %ld000000000", sm.getWattHourMinus(), sm.getTimeMinus());
//         debugView.output(buf);
//         res = influx.write(buf);
//         debugView.output(res);
//         if (rcvEv29)
//         {
//             // EVENT29受信からEVENT25受信しなかったら、再接続開始
//             xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);
//             rcvEv29 = false;
//         }
//         break;
//     case RCV_CODE::EVENT_25:
//         // 接続完了 → 要求コマンドを開始
//         sm.setConnectState(CONNECT_STATE::CONNECTED);
//         Serial.println("EV25");
//         sd_log.out("EV25");
//         sm.request();
//         rcvEv29 = false;
//         break;
//     case RCV_CODE::EVENT_26:
//         // セッション切断要求 → 再接続開始
//     case RCV_CODE::EVENT_27:
//         // PANA セッションの終了に成功 → 再接続開始
//     case RCV_CODE::EVENT_28:
//         // PANA セッションの終了要求に対する応答がなくタイムアウトした(セッションは終了) → 再接続開始
//         Serial.println("EV26-28");
//         sd_log.out("EV26-28");
//         xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);
//         break;
//     case RCV_CODE::EVENT_29:
//         // セッション期限切れ → 送信を止め、'EVENT 25'を待つ
//         sm.setConnectState(CONNECT_STATE::CONNECTING);
//         Serial.println("EV29");
//         sd_log.out("EV29");
//         rcvEv29 = true;
//         break;
//     case RCV_CODE::TIMEOUT:
//     default:
//         break;
//     }
// }

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

void getServer()
{
}

BME280<> BMESensor; // instantiate sensor
void updateBme(boolean withInflux)
{
    while (true)
    {
        BMESensor.refresh(); // read current sensor data
        float p = BMESensor.pressure;
        if (p >= 90000.0)
            break;
        Serial.println(p);
        delay(100);
    }
    dataStore.setTemperature(BMESensor.temperature);
    dataStore.setHumidity(BMESensor.humidity);
    dataStore.setPressure(BMESensor.pressure / 100.0F);
    if (withInflux)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "room temp=%.1f,hum=%.1f,press=%.1f", BMESensor.temperature, BMESensor.humidity, BMESensor.pressure / 100.0F);
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
    // sm.setDebugView(&debugView);
    // sm.init();
    // preState = sm.getConnectState();
    btnA.enable("JOIN");
    btnC.enable(viewController.getNextKey());

    // LAN
    btnB.disable("NTP");
    nw_init();

    // Sensor
    Wire.begin(GPIO_NUM_21, GPIO_NUM_22); // initialize I2C that connects to sensor
#ifndef TEST
    BMESensor.begin(); // initalize bme280 sensor
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
    //     CONNECT_STATE curState = sm.getConnectState();
    //     if (curState != preState)
    //     {
    //         // スマートメータ 状態変化
    //         switch (curState)
    //         {
    //         case CONNECT_STATE::CONNECTED:
    //             btnA.enable("TERM");
    //             break;
    //         case CONNECT_STATE::DISCONNECTED:
    //             btnA.enable("JOIN");
    //             break;
    //         case CONNECT_STATE::CONNECTING:
    //             btnA.disable("CONNECTING");
    //             for (int i = 0; i < 7; i++)
    //                 sd_log.out(sm.getScnannedParam(i));
    //             break;
    //         case CONNECT_STATE::SCANNING:
    //             btnA.disable("SCANNING");
    //             break;
    //         }
    //     }
    // #ifndef TEST
    //     if (curState == CONNECT_STATE::CONNECTED || curState == CONNECT_STATE::CONNECTING)
    //     {
    //         bp35c0_polling();
    //     }
    // #else
    //     bp35c0_polling();
    // #endif
    //     preState = curState;

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
        // if (curState == CONNECT_STATE::CONNECTED)
        // {
        //     // 要求コマンド送信
        //     sm.request();
        // }
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
    // if (bSma)
    // {
    //     dataStore.setPower(smartmeter.getPower());
    //     dataStore.setWattHourPlus(smartmeter.getWattHourPlus(), smartmeter.getTimePlus() + TIMEZONE);
    //     dataStore.setWattHourMinus(smartmeter.getWattHourMinus(), smartmeter.getTimeMinus() + TIMEZONE);
    //     snprintf(buf, sizeof(buf), "power value=%ld", smartmeter.getPower());
    //     debugView.output(buf);
    //     snprintf(buf, sizeof(buf), "+power value=%.1f %ld000000000", smartmeter.getWattHourPlus(), smartmeter.getTimePlus());
    //     debugView.output(buf);
    //     snprintf(buf, sizeof(buf), "-power value=%.1f %ld000000000", smartmeter.getWattHourMinus(), smartmeter.getTimeMinus());
    //     debugView.output(buf);
    //     bSma = false;
    // }

    d = cur - preView;
    if (d < 0)
        d += ULONG_MAX;
    if (d >= VIEW_INTERVAL)
    {
        preView = cur;
        headView.update();
        viewController.update();
    }
    if (btnA.isEnable() && btnA.getButton()->wasPressed())
    {
        // switch (curState)
        // {
        // case CONNECT_STATE::CONNECTED:
        //     // 切断
        //     sm.disconnect();
        //     break;
        // case CONNECT_STATE::DISCONNECTED:
        //     // 接続
        //     xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);
        //     break;
        // case CONNECT_STATE::CONNECTING:
        //     break;
        // case CONNECT_STATE::SCANNING:
        //     break;
        // }
    }
    else if (btnB.isEnable() && btnB.getButton()->wasPressed())
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
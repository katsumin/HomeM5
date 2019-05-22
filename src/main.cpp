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
#include "NtpClient.h"
#include "EchonetUdp.h"
#include "Ecocute.h"
#include "Aircon.h"
#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 26

// instances
FunctionButton btnA(&M5.BtnA);
FunctionButton btnB(&M5.BtnB);
DebugView debugView(0, 100, 320, SCROLL_SIZE * 10);
InfluxDb influx(INFLUX_SERVER, INFLUX_DB);
SmartMeter sm;

/*
スマートメータ接続タスク
*/
void bp35c0_monitoring_task(void *arm)
{
  // bJoined = false;
  Serial.println("task start");
  // bTaskRunning = true;
  // スマートメータ接続
  sm.disconnect();
  // delay(1000);
  sm.connect(PWD, BID);

  Serial.println("task stop");
  vTaskDelete(NULL);
}

void bp35c0_polling()
{
  int res;
  char buf[256];
  RCV_CODE code = sm.polling(buf, sizeof(buf));
  switch (code)
  {
  case RCV_CODE::ERXUDP:
    // UDP受信
    Serial.println("RXUDP");
    break;
  case RCV_CODE::ERXUDP_E7:
    view.setPower(sm.getPower());
    snprintf(buf, sizeof(buf), "power value=%ld", sm.getPower());
    debugView.output(buf);
    res = influx.write(buf);
    debugView.output(res);
    break;
  case RCV_CODE::ERXUDP_EAB:
    view.setWattHourPlus(sm.getWattHourPlus());
    view.setWattHoueMinus(sm.getWattHourMinus());
    snprintf(buf, sizeof(buf), "+power value=%.1f %ld000000000", sm.getWattHourPlus(), sm.getTimePlus());
    debugView.output(buf);
    res = influx.write(buf);
    debugView.output(res);
    snprintf(buf, sizeof(buf), "-power value=%.1f %ld000000000", sm.getWattHourMinus(), sm.getTimeMinus());
    debugView.output(buf);
    res = influx.write(buf);
    debugView.output(res);
    break;
  case RCV_CODE::EVENT_25:
    // 接続完了 → 要求コマンドを開始
    sm.setConnectState(CONNECT_STATE::CONNECTED);
    Serial.println("EV25");
    sm.request();
    break;
  case RCV_CODE::EVENT_26:
    // セッション切断要求 → 再接続開始
  case RCV_CODE::EVENT_27:
    // PANA セッションの終了に成功 → 再接続開始
  case RCV_CODE::EVENT_28:
    // PANA セッションの終了要求に対する応答がなくタイムアウトした(セッションは終了) → 再接続開始
    Serial.println("EV26-28");
    xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);
    break;
  case RCV_CODE::EVENT_29:
    // セッション期限切れ → 送信を止め、'EVENT 25'を待つ
    sm.setConnectState(CONNECT_STATE::CONNECTING);
    Serial.println("EV29");
    break;
  case RCV_CODE::TIMEOUT:
  default:
    break;
  }
}

boolean bEco = false;
void ecocuteCallback(Ecocute *ec)
{
  Serial.println("Ecocute callback");
  bEco = true;
}

boolean bAir = false;
void airconCallback(Aircon *ac)
{
  Serial.println("Aircon callback");
  bAir = true;
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
    Ethernet.begin(mac);
    Serial.println("Ethernet connected");
    Serial.println("IP address: ");
    addr = Ethernet.localIP();
    influx.setEthernet(true);
    snprintf(buf, sizeof(buf), "%s(Ethernet)", addr.toString().c_str());
    view.setNwType("Ethernet");
    udpNtp = new EthernetUDP();
    udpEchonet = new EthernetUDP();
  }
  else
  {
    // WiFi
    wm.addAP(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WIFI");
    while (wm.run() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(100);
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    addr = WiFi.localIP();
    influx.setEthernet(false);
    snprintf(buf, sizeof(buf), "%s(WiFi)", addr.toString().c_str());
    view.setNwType("WiFi");
    udpNtp = new WiFiUDP();
    udpEchonet = new WiFiUDP();
  }
  Serial.println(buf);
  view.setIpAddress(addr);

  // NTP
  ntp.init(udpNtp, NTP_SERVER, random(10000, 19999));
  // echonet
  echonetUdp.init(udpEchonet);
  // ecocute
  ecocute.init(&echonetUdp, ECOCUTE_ADDRESS, ecocuteCallback);
  // aircon
  aircon.init(&echonetUdp, ECOCUTE_ADDRESS, airconCallback);
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
  view.setTemperature(BMESensor.temperature);
  view.setHumidity(BMESensor.humidity);
  view.setPressure(BMESensor.pressure / 100.0F);
  if (withInflux)
  {
    char buf[64];
    snprintf(buf, sizeof(buf), "room temp=%.1f,hum=%.1f,press=%.1f", BMESensor.temperature, BMESensor.humidity, BMESensor.pressure / 100.0F);
    debugView.output(buf);
    int res = influx.write(buf);
    debugView.output(res);
  }
}

CONNECT_STATE preState;
void setup()
{
  M5.begin();

  //  View
  M5.Lcd.clear();
  view.init();

  Serial.begin(115200);

  // スマートメータ
  sm.setDebugView(&debugView);
  sm.init();
  preState = sm.getConnectState();
  btnA.enable("JOIN");

  // LAN
  nw_init();
  ntp.update();
  btnB.disable("NTP");

  // Sensor
  Wire.begin(GPIO_NUM_21, GPIO_NUM_22); // initialize I2C that connects to sensor
  BMESensor.begin();                    // initalize bme280 sensor
  updateBme(true);

  delay(500);
  ecocute.request();
  aircon.request();
}

boolean bNtp = true;
unsigned long preMeas = millis();
unsigned long preView = preMeas;
#define INTERVAL (120 * 1000)
#define VIEW_INTERVAL (1000)
void loop()
{
  CONNECT_STATE curState = sm.getConnectState();
  if (curState != preState)
  {
    // スマートメータ 状態変化
    switch (curState)
    {
    case CONNECT_STATE::CONNECTED:
      btnA.enable("TERM");
      break;
    case CONNECT_STATE::DISCONNECTED:
      btnA.enable("JOIN");
      break;
    case CONNECT_STATE::CONNECTING:
      btnA.disable("CONNECTING");
      break;
    case CONNECT_STATE::SCANNING:
      btnA.disable("SCANNING");
      break;
    }
  }
  if (curState == CONNECT_STATE::CONNECTED || curState == CONNECT_STATE::CONNECTING)
  {
    bp35c0_polling();
  }
  preState = curState;

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
    updateBme(true);

    // smartmeter
    if (curState == CONNECT_STATE::CONNECTED)
    {
      // 要求コマンド送信
      sm.request();
    }
    // ecocute
    ecocute.request();
    // aircon
    aircon.request();
  }

  char buf[64];
  if (bEco)
  {
    view.setEcoPower(ecocute.getPower());
    view.setEcoTank(ecocute.getTank());
    //text = "ecocute power={0},powerSum={1},tank={2} {3}\n".format(edt1, edt2, edt3, timestamp)
    snprintf(buf, sizeof(buf), "ecocute power=%ld,powerSum=%ld,tank=%ld", ecocute.getPower(), ecocute.getTotalPower(), ecocute.getTank());
    debugView.output(buf);
    int res = influx.write(buf);
    debugView.output(res);
    bEco = false;
  }
  if (bAir)
  {
    view.setAirPower(aircon.getPower());
    view.setAirTempOut(aircon.getTempOut());
    view.setAirTempRoom(aircon.getTempRoom());
    //text = "aircon power={0},tempOut={1},tempRoom={2} {3}\n".format(edt1, edt2, edt3, timestamp)
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
    view.update();
  }
  if (btnA.getButton()->wasPressed())
  {
    switch (curState)
    {
    case CONNECT_STATE::CONNECTED:
      // 切断
      sm.disconnect();
      break;
    case CONNECT_STATE::DISCONNECTED:
      // 接続
      xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);
      break;
    case CONNECT_STATE::CONNECTING:
      break;
    case CONNECT_STATE::SCANNING:
      break;
    }
  }
  if (bNtp)
  {
    if (ntp.getMillis() > 0)
    {
      bNtp = false;
      time_t epoch = ntp.getEpocTime() + (millis() - ntp.getMillis()) / 1000;
      Serial.println(epoch);
      view.setTime(epoch);
      btnB.enable("NTP");
    }
  }
  else
  {
    if (btnB.getButton()->wasReleased())
    {
      ntp.update();
      bNtp = true;
      btnB.disable("NTP");
    }
  }
  delay(10);
  M5.update();
}
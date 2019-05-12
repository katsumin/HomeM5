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
#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 26
// #define WIDTH (60)
// #define POS_A_X (36)

// #define TEXT_HEIGHT (16)
FunctionButton btnA(&M5.BtnA);
DebugView debugView(0, 100, 320, SCROLL_SIZE * 10);
InfluxDb influx(INFLUX_SERVER, INFLUX_DB);
SmartMeter sm;
// int16_t fontWidth;
// int16_t fontHeight;

// void button(int32_t x_pos, const char *label)
// {
//   M5.Lcd.setTextSize(1);
//   M5.Lcd.setTextColor(WHITE);
//   uint32_t y_pos = M5.Lcd.height() - TEXT_HEIGHT;
//   M5.Lcd.fillRect(x_pos, y_pos - 1, WIDTH, TEXT_HEIGHT, BLACK);
//   M5.Lcd.setTextColor(WHITE);
//   M5.Lcd.setCursor(x_pos + WIDTH / 2 - M5.Lcd.textWidth(label) / 2, y_pos + 3);
//   M5.Lcd.print(label);
//   M5.Lcd.drawRoundRect(x_pos, y_pos - 1, WIDTH, TEXT_HEIGHT, 2, WHITE);
// }
class MainView
{
private:
  // 気温
  float _temperature = 0;
  // 湿度
  float _humidity = 0;
  // 気圧
  float _pressure = 0;
  // 瞬時電力
  long _power = 0;
  // 正方向積算電力量
  float _wattHourPlus = 0;
  // 負方向積算電力量
  float _wattHourMinus = 0;
  // 1文字幅
  int16_t _fontWidth;
  // 1文字高
  int16_t _fontHeight;

public:
  inline void setTemperature(float temp) { _temperature = temp; };
  inline void setHumidity(float hum) { _humidity = hum; };
  inline void setPressure(float press) { _pressure = press; };
  inline void setPower(long power) { _power = power; };
  inline void setWattHourPlus(float plus) { _wattHourPlus = plus; };
  inline void setWattHoueMinus(float minus) { _wattHourMinus = minus; };
  void init()
  {
    M5.Lcd.setTextSize(2);
    _fontHeight = M5.Lcd.fontHeight(1);
    _fontWidth = M5.Lcd.textWidth(" ");
    M5.Lcd.drawString("temperature:           C", 0, 0);
    M5.Lcd.drawString("humidity   :           %H", 0, _fontHeight);
    M5.Lcd.drawString("pressure   :           hPa", 0, _fontHeight * 2);
    M5.Lcd.drawString("power      :           W", 0, _fontHeight * 3);
    M5.Lcd.drawString("watt-hour +:           kWh", 0, _fontHeight * 4);
    M5.Lcd.drawString("watt-hour -:           kWh", 0, _fontHeight * 5);
  }
  void update()
  {
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(YELLOW);
    int x = 13 * _fontWidth;
    M5.Lcd.fillRect(x, 0, _fontWidth * 9, _fontHeight * 6, BLACK);
    char buf[16];
    x = 16 * _fontWidth;
    snprintf(buf, sizeof(buf), "%6.1f", _temperature);
    M5.Lcd.drawString(buf, x, _fontHeight * 0);
    snprintf(buf, sizeof(buf), "%6.1f", _humidity);
    M5.Lcd.drawString(buf, x, _fontHeight * 1);
    snprintf(buf, sizeof(buf), "%6.1f", _pressure);
    M5.Lcd.drawString(buf, x, _fontHeight * 2);
    x = 13 * _fontWidth;
    snprintf(buf, sizeof(buf), "%9ld", _power);
    M5.Lcd.drawString(buf, x, _fontHeight * 3);
    snprintf(buf, sizeof(buf), "%9.1f", _wattHourPlus);
    M5.Lcd.drawString(buf, x, _fontHeight * 4);
    snprintf(buf, sizeof(buf), "%9.1f", _wattHourMinus);
    M5.Lcd.drawString(buf, x, _fontHeight * 5);
  };
};

MainView view;
// std::queue<String> influxQueue;
// static boolean sendEnable = false;
void bp35c0_polling()
{
  int res;
  // int x = 13 * fontWidth;
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
    // snprintf(buf, sizeof(buf), "%9ld", sm.getPower());
    // M5.Lcd.setTextSize(2);
    // M5.Lcd.setTextColor(YELLOW);
    // M5.Lcd.fillRect(x, fontHeight * 3, fontWidth * 9, fontHeight, BLACK);
    // M5.Lcd.drawString(buf, x, fontHeight * 3);
    snprintf(buf, sizeof(buf), "power value=%ld", sm.getPower());
    debugView.output(buf);
    res = influx.write(buf);
    debugView.output(res);
    // influxQueue.push(String(buf));
    break;
  case RCV_CODE::ERXUDP_EAB:
    view.setWattHourPlus(sm.getWattHourPlus());
    view.setWattHoueMinus(sm.getWattHourMinus());
    // snprintf(buf, sizeof(buf), "%9.1f", sm.getWattHourPlus());
    // M5.Lcd.setTextSize(2);
    // M5.Lcd.setTextColor(YELLOW);
    // M5.Lcd.fillRect(x, fontHeight * 4, fontWidth * 9, fontHeight, BLACK);
    // M5.Lcd.drawString(buf, x, fontHeight * 4);
    // snprintf(buf, sizeof(buf), "%9.1f", sm.getWattHourMinus());
    // M5.Lcd.fillRect(x, fontHeight * 5, fontWidth * 9, fontHeight, BLACK);
    // M5.Lcd.drawString(buf, x, fontHeight * 5);
    snprintf(buf, sizeof(buf), "+power value=%.1f %ld000000000", sm.getWattHourPlus(), sm.getTimePlus());
    debugView.output(buf);
    res = influx.write(buf);
    debugView.output(res);
    // influxQueue.push(String(buf));
    snprintf(buf, sizeof(buf), "-power value=%.1f %ld000000000", sm.getWattHourMinus(), sm.getTimeMinus());
    debugView.output(buf);
    res = influx.write(buf);
    debugView.output(res);
    // influxQueue.push(String(buf));
    break;
  case RCV_CODE::EVENT_25:
    // 接続完了 → 要求コマンドを開始
    Serial.println("EV25");
    // sendEnable = true;
    sm.request();
    break;
  case RCV_CODE::EVENT_26:
    // セッション切断要求 → 再接続開始
  case RCV_CODE::EVENT_27:
    // PANA セッションの終了に成功 → 再接続開始
  case RCV_CODE::EVENT_28:
    // PANA セッションの終了要求に対する応答がなくタイムアウトした(セッションは終了) → 再接続開始
    Serial.println("EV26-28");
    sm.disconnect();
    delay(1000);
    sm.connect(PWD, BID);
    // sendEnable = false;
    break;
  case RCV_CODE::EVENT_29:
    // セッション期限切れ → 送信を止め、'EVENT 25'を待つ
    Serial.println("EV29");
    // sendEnable = false;
    break;
  case RCV_CODE::TIMEOUT:
  default:
    break;
  }
}

#define SEND_INTERVAL (120 * 1000)
// static boolean reqTerm = false;
// boolean bTaskRunning = false;
// boolean bJoined = false;
/*
BP35C0ハンドリング・タスク
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

  // unsigned long pre = millis();
  // while (true)
  // {
  //   unsigned long cur = millis();
  //   if (sendEnable && cur - pre > SEND_INTERVAL)
  //   {
  //     pre = millis();
  //     // 要求コマンド送信
  //     sm.request();
  //   }
  //   if (reqTerm)
  //   {
  //     // 停止コマンド送信
  //     reqTerm = false;
  //     sm.disconnect();
  //     // タスク終了
  //     break;
  //   }
  //   bp35c0_polling();

  //   vTaskDelay(10 / portTICK_RATE_MS);
  // }
  Serial.println("task stop");
  // bTaskRunning = false;
  // bJoined = true;
  // button(POS_A_X, "TERM");
  vTaskDelete(NULL);
}

// WiFiClient wc;
// EthernetClient ec;
WiFiMulti wm;
void nw_init()
{
  Serial.println();
  SPI.begin(SCK, MISO, MOSI, -1);
  Ethernet.init(CS); // Ethernet/Ethernet2
  // Ethernet.setCsPin(CS); // Ethernet3
  IPAddress addr;
  char *nw_type;
  if (Ethernet.linkStatus() == LinkON) // Ethernet
  // if (Ethernet.begin(mac) == 1) // Ethernet3
  {
    // Ethernet
    Ethernet.begin(mac);
    Serial.println("Ethernet connected");
    Serial.println("IP address: ");
    addr = Ethernet.localIP();

    influx.setEthernet(true);
    // influx.setClient(&ec);
    nw_type = "Ethernet";
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
    // influx.setClient(&wc);
    nw_type = "WiFi";
  }
  char buf[64];
  snprintf(buf, sizeof(buf), "%s(%s)", addr.toString().c_str(), nw_type);
  Serial.println(buf);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE);
  int32_t x_pos = 130;
  int32_t y_pos = M5.Lcd.height() - TEXT_HEIGHT + 3;
  // M5.Lcd.fillRect(x_pos, y_pos - 1, WIDTH, TEXT_HEIGHT, BLACK);
  M5.Lcd.drawString(buf, x_pos, y_pos);
}

BME280<> BMESensor; // instantiate sensor
void updateBme()
{
  BMESensor.refresh(); // read current sensor data
  view.setTemperature(BMESensor.temperature);
  view.setHumidity(BMESensor.humidity);
  view.setPressure(BMESensor.pressure / 100.0F);
  char buf[64];
  snprintf(buf, sizeof(buf), "room temp=%.1f,hum=%.1f,press=%.1f", BMESensor.temperature, BMESensor.humidity, BMESensor.pressure / 100.0F);
  // Serial.println(buf);
  debugView.output(buf);
  int res = influx.write(buf);
  debugView.output(res);
  // Serial.println(res);
  // int x = 16 * fontWidth;
  // M5.Lcd.setTextSize(2);
  // M5.Lcd.setTextColor(YELLOW);
  // snprintf(buf, sizeof(buf), "%6.1f", BMESensor.temperature);
  // M5.Lcd.fillRect(x, 0, fontWidth * 6, fontHeight, BLACK);
  // M5.Lcd.drawString(buf, x, 0);
  // snprintf(buf, sizeof(buf), "%6.1f", BMESensor.humidity);
  // M5.Lcd.fillRect(x, fontHeight, fontWidth * 6, fontHeight, BLACK);
  // M5.Lcd.drawString(buf, x, fontHeight);
  // snprintf(buf, sizeof(buf), "%6.1f", BMESensor.pressure / 100.0F);
  // M5.Lcd.fillRect(x, fontHeight * 2, fontWidth * 6, fontHeight, BLACK);
  // M5.Lcd.drawString(buf, x, fontHeight * 2);
}

// void updateSmartMeter()
// {
//   char buf[64];
//   int x = 13 * fontWidth;
//   snprintf(buf, sizeof(buf), "%9ld", sm.getPower());
//   M5.Lcd.setTextSize(2);
//   M5.Lcd.setTextColor(YELLOW);
//   M5.Lcd.fillRect(x, fontHeight * 3, fontWidth * 9, fontHeight, BLACK);
//   M5.Lcd.drawString(buf, x, fontHeight * 3);
// }

CONNECT_STATE preState;
void setup()
{
  M5.begin();
  M5.Lcd.clear();
  view.init();

  sm.setDebugView(&debugView);
  Serial.begin(115200);
  sm.init();
  preState = sm.getConnectState();

  nw_init();
  Wire.begin(GPIO_NUM_21, GPIO_NUM_22); // initialize I2C that connects to sensor
  BMESensor.begin();                    // initalize bme280 sensor

  // M5.Lcd.setTextSize(2);
  // M5.Lcd.setTextColor(WHITE);
  // fontHeight = M5.Lcd.fontHeight(1);
  // fontWidth = M5.Lcd.textWidth(" ");

  // M5.Lcd.drawString("temperature:           C", 0, 0);
  // M5.Lcd.drawString("humidity   :           %H", 0, fontHeight);
  // M5.Lcd.drawString("pressure   :           hPa", 0, fontHeight * 2);
  // M5.Lcd.drawString("power      :           W", 0, fontHeight * 3);
  // M5.Lcd.drawString("watt-hour +:           kWh", 0, fontHeight * 4);
  // M5.Lcd.drawString("watt-hour -:           kWh", 0, fontHeight * 5);
  updateBme();
  // button(POS_A_X, "JOIN");
  btnA.set("JOIN");
}

unsigned long preMeas = millis();
unsigned long preView = preMeas;
#define INTERVAL (120 * 1000)
#define VIEW_INTERVAL (1000)
// long diff = 0;
void loop()
{
  // if (!influxQueue.empty())
  // {
  //   String s = influxQueue.front();
  //   debugView.output(s.c_str());
  //   int res = influx.write(s.c_str());
  //   debugView.output(res);
  //   influxQueue.pop();
  // }
  CONNECT_STATE curState = sm.getConnectState();
  if (curState != preState)
  {
    // スマートメータ 状態変化
    switch (curState)
    {
    case CONNECT_STATE::CONNECTED:
      btnA.set("TERM");
      // 要求コマンド送信
      // preMeas = millis();
      // sm.request();
      break;
    case CONNECT_STATE::DISCONNECTED:
      btnA.set("JOIN");
      break;
    case CONNECT_STATE::CONNECTING:
      break;
    }
  }
  preState = curState;
  // if (M5.BtnA.wasPressed())
  // {
  //   switch (curState)
  //   {
  //   case CONNECT_STATE::CONNECTED:
  //     // 切断
  //     sm.disconnect();
  //     // button(POS_A_X, "JOIN");
  //     break;
  //   case CONNECT_STATE::DISCONNECTED:
  //     // 接続
  //     xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);
  //     btnA.set("----");
  //     break;
  //   case CONNECT_STATE::CONNECTING:
  //     break;
  //   }
  // }

  // if (bJoined)
  // {
  //   // 停止コマンド送信
  //   sm.disconnect();
  //   button(POS_A_X, "JOIN");
  // }
  // else
  // {
  //   // 接続
  //   sm.setConnectState(CONNECT_STATE::CONNECTING);
  //   xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);
  // }

  // if (bTaskRunning)
  // {
  //   // 切断
  //   reqTerm = true;
  //   // 停止コマンド送信
  //   sm.disconnect();
  //   button(POS_A_X, "JOIN");
  // }
  // else
  // {
  //   // 接続
  //   // bTaskRunning = true;
  //   xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);
  //   // button(POS_A_X, "TERM");
  // }
  // }

  long cur = millis();
  long d = cur - preMeas;
  if (d < 0)
    d += ULONG_MAX;
  if (curState == CONNECT_STATE::CONNECTED)
  {
    bp35c0_polling();
    if (d >= INTERVAL) // debug
    {
      preMeas = cur;

      char buf[64];
      snprintf(buf, sizeof(buf), "%ld -> %ld, memory(%d)", preMeas, d, xPortGetFreeHeapSize());
      debugView.output(buf);

      // BME280
      updateBme();

      // smartmeter
      // 要求コマンド送信
      sm.request();
    }
  }
  d = cur - preView;
  if (d < 0)
    d += ULONG_MAX;
  if (d >= VIEW_INTERVAL)
  {
    preView = cur;
    view.update();
  }
  if (M5.BtnA.wasPressed())
  {
    switch (curState)
    {
    case CONNECT_STATE::CONNECTED:
      // 切断
      sm.disconnect();
      // button(POS_A_X, "JOIN");
      break;
    case CONNECT_STATE::DISCONNECTED:
      // 接続
      xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);
      btnA.set("----");
      break;
    case CONNECT_STATE::CONNECTING:
      break;
    }
  }
  // if (d >= SEND_INTERVAL && bJoined)
  // {
  //   // 要求コマンド送信
  //   sm.request();
  // }
  // if (sm.getConnectState() == CONNECT_STATE::CONNECTED)
  //   bp35c0_polling();

  delay(10);
  M5.update();
}
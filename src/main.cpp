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
#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 26

#define TEXT_HEIGHT (16)
DebugView debugView(0, 100, 320, TEXT_HEIGHT * 7 - 1);
InfluxDb influx(INFLUX_SERVER, INFLUX_DB);
SmartMeter sm;
int16_t fontWidth;
int16_t fontHeight;

static boolean sendEnable = true;
void bp35c0_polling()
{
  int res;
  int x = 13 * fontWidth;
  char buf[256];
  RCV_CODE code = sm.polling(buf, sizeof(buf));
  switch (code)
  {
  case RCV_CODE::ERXUDP:
    // UDP受信
    Serial.println("RXUDP");
    break;
  case RCV_CODE::ERXUDP_E7:
    snprintf(buf, sizeof(buf), "%9ld", sm.getPower());
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.fillRect(x, fontHeight * 3, fontWidth * 9, fontHeight, BLACK);
    M5.Lcd.drawString(buf, x, fontHeight * 3);
    snprintf(buf, sizeof(buf), "power value=%ld", sm.getPower());
    debugView.output(buf);
    res = influx.write(buf);
    debugView.output(res);
    break;
  case RCV_CODE::ERXUDP_EAB:
    snprintf(buf, sizeof(buf), "%9.1f", sm.getWattHourPlus());
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.fillRect(x, fontHeight * 4, fontWidth * 9, fontHeight, BLACK);
    M5.Lcd.drawString(buf, x, fontHeight * 4);
    snprintf(buf, sizeof(buf), "%9.1f", sm.getWattHourMinus());
    M5.Lcd.fillRect(x, fontHeight * 5, fontWidth * 9, fontHeight, BLACK);
    M5.Lcd.drawString(buf, x, fontHeight * 5);
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
    Serial.println("EV25");
    sendEnable = true;
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
    break;
  case RCV_CODE::EVENT_29:
    // セッション期限切れ → 送信を止め、'EVENT 25'を待つ
    Serial.println("EV29");
    sendEnable = false;
    break;
  case RCV_CODE::TIMEOUT:
  default:
    break;
  }
}

#define SEND_INTERVAL (120 * 1000)
static boolean reqTerm = false;
boolean bTaskRunning = false;
/*
BP35C0ハンドリング・タスク
*/
void bp35c0_monitoring_task(void *arm)
{
  Serial.println("task start");
  bTaskRunning = true;
  // スマートメータ接続
  sm.disconnect();
  delay(1000);
  sm.connect(PWD, BID);

  unsigned long pre = millis();
  while (true)
  {
    unsigned long cur = millis();
    if (sendEnable && cur - pre > SEND_INTERVAL)
    {
      pre = millis();
      // 要求コマンド送信
      sm.request();
    }
    if (reqTerm)
    {
      // 停止コマンド送信
      reqTerm = false;
      sm.disconnect();
      // タスク終了
      break;
    }
    bp35c0_polling();

    vTaskDelay(10 / portTICK_RATE_MS);
  }
  Serial.println("task stop");
  bTaskRunning = false;
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
  char buf[64];
  snprintf(buf, sizeof(buf), "room temp=%.1f,hum=%.1f,press=%.1f", BMESensor.temperature, BMESensor.humidity, BMESensor.pressure / 100.0F);
  // Serial.println(buf);
  debugView.output(buf);
  int res = influx.write(buf);
  debugView.output(res);
  // Serial.println(res);
  int x = 16 * fontWidth;
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(YELLOW);
  snprintf(buf, sizeof(buf), "%6.1f", BMESensor.temperature);
  M5.Lcd.fillRect(x, 0, fontWidth * 6, fontHeight, BLACK);
  M5.Lcd.drawString(buf, x, 0);
  snprintf(buf, sizeof(buf), "%6.1f", BMESensor.humidity);
  M5.Lcd.fillRect(x, fontHeight, fontWidth * 6, fontHeight, BLACK);
  M5.Lcd.drawString(buf, x, fontHeight);
  snprintf(buf, sizeof(buf), "%6.1f", BMESensor.pressure / 100.0F);
  M5.Lcd.fillRect(x, fontHeight * 2, fontWidth * 6, fontHeight, BLACK);
  M5.Lcd.drawString(buf, x, fontHeight * 2);
}

void updateSmartMeter()
{
  char buf[64];
  int x = 13 * fontWidth;
  snprintf(buf, sizeof(buf), "%9ld", sm.getPower());
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.fillRect(x, fontHeight * 3, fontWidth * 9, fontHeight, BLACK);
  M5.Lcd.drawString(buf, x, fontHeight * 3);
}

#define WIDTH (60)
#define POS_A_X (36)
void button(int32_t x_pos, const char *label)
{
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE);
  uint32_t y_pos = M5.Lcd.height() - TEXT_HEIGHT;
  M5.Lcd.fillRect(x_pos, y_pos - 1, WIDTH, TEXT_HEIGHT, BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(x_pos + WIDTH / 2 - M5.Lcd.textWidth(label) / 2, y_pos + 3);
  M5.Lcd.print(label);
  M5.Lcd.drawRoundRect(x_pos, y_pos - 1, WIDTH, TEXT_HEIGHT, 2, WHITE);
}

void setup()
{
  M5.begin();
  M5.Lcd.clear();

  sm.setDebugView(&debugView);
  Serial.begin(115200);
  sm.init();

  nw_init();
  Wire.begin(GPIO_NUM_21, GPIO_NUM_22); // initialize I2C that connects to sensor
  BMESensor.begin();                    // initalize bme280 sensor

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  fontHeight = M5.Lcd.fontHeight(1);
  fontWidth = M5.Lcd.textWidth(" ");

  M5.Lcd.drawString("temperature:           C", 0, 0);
  M5.Lcd.drawString("humidity   :           %H", 0, fontHeight);
  M5.Lcd.drawString("pressure   :           hPa", 0, fontHeight * 2);
  M5.Lcd.drawString("power      :           W", 0, fontHeight * 3);
  M5.Lcd.drawString("watt-hour +:           kWh", 0, fontHeight * 4);
  M5.Lcd.drawString("watt-hour -:           kWh", 0, fontHeight * 5);
  updateBme();
  button(POS_A_X, "JOIN");
}

unsigned long pre = ULONG_MAX;
unsigned long cur;
static const unsigned long INTERVAL = 60 * 1000;
long diff = 0;
void loop()
{
  if (M5.BtnA.wasPressed())
  {
    if (bTaskRunning)
    {
      // 切断
      reqTerm = true;
      button(POS_A_X, "JOIN");
    }
    else
    {
      // 接続
      bTaskRunning = true;
      xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);
      button(POS_A_X, "TERM");
    }
  }

  char buf[64];
  if (pre == ULONG_MAX)
  {
    pre = millis();
  }
  cur = millis();
  if (cur - pre > INTERVAL - diff)
  {
    diff = cur - pre;
    snprintf(buf, sizeof(buf), "%ld - %ld = %ld", cur, pre, diff);
    debugView.output(buf);
    diff -= INTERVAL;
    pre = cur;
    updateBme();
  }

  delay(100);
  M5.update();
}
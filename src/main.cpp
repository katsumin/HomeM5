#include <M5Stack.h>
#include "smartmeter.h"

#define PWD "password"
#define BID "bid"

SmartMeter sm;

#define SEND_INTERVAL (60 * 1000 / 1500)
static boolean reqTerm = false;
/*
BP35C0ハンドリング・タスク
*/
void bp35c0_monitoring_task(void *arm)
{
  while (true)
  {
    // スマートメータ接続
    sm.disconnect();
    delay(1000);
    sm.connect(PWD, BID);

    char buf[256];
    int sndCount = 0;
    while (true)
    {
      if (sndCount > 0 && --sndCount == 0)
      {
        // 要求コマンド送信
        sm.request();
        sndCount = SEND_INTERVAL;
      }
      if (reqTerm)
      {
        reqTerm = false;
        sm.disconnect();
        // return;
      }
      boolean rejoin = false;
      RCV_CODE code = sm.polling(buf, sizeof(buf));
      switch (code)
      {
      case RCV_CODE::ERXUDP:
        // UDP受信
        break;
      case RCV_CODE::EVENT_25:
        // 接続完了 → 要求コマンドを開始
        sndCount = SEND_INTERVAL;
        break;
      case RCV_CODE::EVENT_26:
        // セッション切断要求 → 再接続開始
      case RCV_CODE::EVENT_27:
        // PANA セッションの終了に成功 → 再接続開始
      case RCV_CODE::EVENT_28:
        // PANA セッションの終了要求に対する応答がなくタイムアウトした(セッションは終了) → 再接続開始
        rejoin = true;
        sndCount = 0;
        break;
      case RCV_CODE::EVENT_29:
        // セッション期限切れ → 送信を止め、'EVENT 25'を待つ
        sndCount = 0;
        break;
      case RCV_CODE::TIMEOUT:
      default:
        break;
      }
      if (rejoin)
        break;

      vTaskDelay(500 / portTICK_RATE_MS);
    }
  }
}

void setup()
{
  M5.begin();

  Serial.begin(115200);
  sm.init();

  xTaskCreate(&bp35c0_monitoring_task, "bp35c0r", 4096, NULL, 5, NULL);

  M5.Lcd.clear();
  M5.Lcd.setBrightness(0);
}

void loop()
{
  if (M5.BtnA.wasPressed())
  {
    reqTerm = true;
  }
  delay(100);
  M5.update();
}
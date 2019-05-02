#include "smartmeter.h"

void SmartMeter::init()
{
  gpio_set_level(BP35C0_RSTn, 0); // BP35C0 Reset = on
  gpio_set_direction(BP35C0_RSTn, GPIO_MODE_OUTPUT);
  delay(100);                     // 100ms wait
  gpio_set_level(BP35C0_RSTn, 1); // BP35C0 Reset = off
  delay(500);                     // 500ms wait

  Serial2.begin(BP35C0_bps, SERIAL_8N1, 16, 17);
  Serial2.setTimeout(1000);
  // Serial2.println("WOPT 01");
}

void SmartMeter::connect(const char *pwd, const char *bid)
{
  char buf[128];
  while (true)
  {
    Serial.println("join start");

    if (sendCmdAndWaitOk("SKVER") < 0) // SKVER
      continue;
    snprintf(buf, sizeof(buf), "SKSETPWD C %s", pwd);
    if (sendCmdAndWaitOk(buf) < 0) // SKSETPWD
      continue;
    snprintf(buf, sizeof(buf), "SKSETRBID %s", bid);
    if (sendCmdAndWaitOk(buf) < 0) // SKSETRBID
      continue;

    // SKSCAN
    scan();

    snprintf(buf, sizeof(buf), "SKSREG S2 %s", _para[BP35C0_CHANNEL]);
    if (sendCmdAndWaitOk(buf) < 0) // SKSREG S2(Channel)
      continue;
    snprintf(buf, sizeof(buf), "SKSREG S3 %s", _para[BP35C0_PAN_ID]);
    if (sendCmdAndWaitOk(buf) < 0) // SKSREG S3(Pan ID)
      continue;
    sendCmdAndWaitIpv6Address(); // SKLL64(Addr)
    // snprintf(buf, sizeof(buf), "SKJOIN %s", _addr);
    // if (sendCmdAndWaitOk(buf) < 0) // SKJOIN
    //   continue;

    Serial.println("join end");
    break;
  }
}

void SmartMeter::disconnect()
{
  Serial.println("term");
  sendCmdAndWaitOk("SKTERM"); // SKTERM
}

u_char SmartMeter::_cmd_buf[] = {
    0x10,
    0x81, // EHD
    0x00,
    0x01, // TID
    0x05,
    0xff,
    0x01, // SEOJ
    0x02,
    0x88,
    0x01, // DEOJ
    0x62, // ESV(プロパティ値読み出し要求)
    0x01, // OPC(1data)
    0xe7, // EPC()
    0x00, // PDC
};
void SmartMeter::request()
{
  Serial.println("send request");
  char buf[128];
  int len = sizeof(_cmd_buf);
  snprintf(buf, sizeof(buf), "SKSENDTO 1 %s 0E1A 1 0 %04d ", _addr, len);
  Serial2.print(buf);
  for (int i = 0; i < len; i++)
  {
    Serial2.write(_cmd_buf[i]);
  }
}

int SmartMeter::waitOk()
{
  while (1)
  {
    char buf[64];
    size_t len = Serial2.readBytesUntil(0x0a, buf, sizeof(buf));
    if (len > 0)
    {
      buf[len - 1] = 0;
      Serial.println(buf);
      if (strncmp(buf, "OK", 2) == 0)
        return 0;
    }
    else
    {
      return -1;
    }

    vTaskDelay(10 / portTICK_RATE_MS);
  }
}

void SmartMeter::sendCmdAndWaitIpv6Address()
{
  char buf[128];
  snprintf(buf, sizeof(buf), "SKLL64 %s", _para[BP35C0_ADDR]);
  Serial2.println(buf);
  while (1)
  {
    size_t len = Serial2.readBytesUntil(0x0a, _addr, sizeof(_addr));
    if (len > 0)
    {
      _addr[len - 1] = 0;
      Serial.println(_addr);
      if (strncmp(_addr, "SKLL64", strlen("SKLL64")) != 0)
        break;
    }

    vTaskDelay(10 / portTICK_RATE_MS);
  }
}

int SmartMeter::sendCmdAndWaitOk(const char *cmd)
{
  Serial2.println(cmd);
  return waitOk();
}

void SmartMeter::scan()
{
  char buf[128];
  boolean scaned = false;
  while (!scaned)
  {
    if (sendCmdAndWaitOk("SKSCAN 2 FFFFFFFF 6 0") < 0) // SKSCAN
      continue;
    Serial.println("wait start");
    delay(10000);
    while (true)
    {
      size_t len = Serial2.readBytesUntil(0x0a, buf, sizeof(buf));
      Serial.printf("%d :", len);
      Serial.println("wait end");
      if (len > 0)
      {
        buf[len - 1] = 0;
        Serial.println(buf);
        if (strncmp(buf, "EVENT 22", strlen("EVENT 22")) == 0)
          break;
        else if (strncmp(buf, "EPANDESC", strlen("EPANDESC")) == 0)
          scaned = true;
        else if (strncmp(buf, "  ", strlen("  ")) == 0)
        {
          int i = 0;
          for (; i < len; i++)
          {
            if (buf[i] == ':')
            {
              buf[i++] = 0;
              break;
            }
          }
          if (strncmp(&buf[2], "Channel Page", strlen("Channel Page")) == 0)
            strcpy(_para[BP35C0_CHANNEL_PAGE], &buf[i]);
          else if (strncmp(&buf[2], "Channel", strlen("Channel")) == 0)
            strcpy(_para[BP35C0_CHANNEL], &buf[i]);
          else if (strncmp(&buf[2], "Pan ID", strlen("Pan ID")) == 0)
            strcpy(_para[BP35C0_PAN_ID], &buf[i]);
          else if (strncmp(&buf[2], "Addr", strlen("Addr")) == 0)
            strcpy(_para[BP35C0_ADDR], &buf[i]);
          else if (strncmp(&buf[2], "LQI", strlen("LQI")) == 0)
            strcpy(_para[BP35C0_LQI], &buf[i]);
          else if (strncmp(&buf[2], "Side", strlen("Side")) == 0)
            strcpy(_para[BP35C0_SIDE], &buf[i]);
          else if (strncmp(&buf[2], "PairID", strlen("PaidID")) == 0)
            strcpy(_para[BP35C0_PAIR_ID], &buf[i]);
        }
      }
      vTaskDelay(10 / portTICK_RATE_MS);
    }
  }
  for (int i = 0; i < 7; i++)
  {
    Serial.printf("%d :", i);
    Serial.println(_para[i]);
  }
}

RCV_CODE SmartMeter::polling(char *data, size_t size)
{
  RCV_CODE r = TIMEOUT;
  char buf[128];
  size_t len = Serial2.readBytesUntil(0x0a, buf, sizeof(buf));
  if (len > 0)
  {
    buf[len - 1] = 0;
    Serial.println(buf);
    if (strncmp(buf, "ERXUDP", strlen("ERXUDP")) == 0)
    {
      // UDP受信
      r = ERXUDP;
    }
    else if (strncmp(buf, "EVENT 25", strlen("EVENT 25")) == 0)
      // 要求コマンドを開始
      r = EVENT_25;
    else if (strncmp(buf, "EVENT 26", strlen("EVENT 26")) == 0)
      // セッション切断要求 → 再接続開始
      r = EVENT_26;
    else if (strncmp(buf, "EVENT 27", strlen("EVENT 27")) == 0)
      // PANA セッションの終了に成功 → 再接続開始
      r = EVENT_27;
    else if (strncmp(buf, "EVENT 28", strlen("EVENT 28")) == 0)
      // PANA セッションの終了要求に対する応答がなくタイムアウトした(セッションは終了) → 再接続開始
      r = EVENT_28;
    else if (strncmp(buf, "EVENT 29", strlen("EVENT 29")) == 0)
      // セッション期限切れ → 送信を止め、'EVENT 25'を待つ
      r = EVENT_29;
  }

  return r;
}
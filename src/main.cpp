#include <M5Stack.h>

#define BP35C0_RSTn GPIO_NUM_5
#define BP35C0_bps 115200
#define PWD "password"
#define BID "bid"
#define BP35C0_CHANNEL 0
#define BP35C0_CHANNEL_PAGE 1
#define BP35C0_PAN_ID 2
#define BP35C0_ADDR 3
#define BP35C0_LQI 4
#define BP35C0_SIDE 5
#define BP35C0_PAIR_ID 6

static u_char cmd_buf[] = {
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
static char para[7][20];
static char addr[64];

/*
要求コマンド送信
*/
void sendReqCmd()
{
  Serial.println("send request");
  char buf[128];
  int len = sizeof(cmd_buf);
  snprintf(buf, sizeof(buf), "SKSENDTO 1 %s 0E1A 1 0 %04d ", addr, len);
  Serial2.print(buf);
  for (int i = 0; i < len; i++)
  {
    Serial2.write(cmd_buf[i]);
  }
}

/*
OK文字列の受信待ち
[引数]
 なし
[戻り値]
 0: OKを受信した
 -1: OKを受信せず、タイムアウトした
*/
int waitOk()
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

/*
IPV6アドレス取得
[引数]
 なし
[戻り値]
 void
[副作用]
 ・IPV6アドレスが、addr配列に格納される。
*/
void sendCmdAndWaitIpv6Address()
{
  char buf[128];
  snprintf(buf, sizeof(buf), "SKLL64 %s", para[BP35C0_ADDR]);
  Serial2.println(buf);
  while (1)
  {
    size_t len = Serial2.readBytesUntil(0x0a, addr, sizeof(addr));
    if (len > 0)
    {
      addr[len - 1] = 0;
      Serial.println(addr);
      if (strncmp(addr, "SKLL64", strlen("SKLL64")) != 0)
        break;
    }

    vTaskDelay(10 / portTICK_RATE_MS);
  }
}

/*
コマンド送信
[引数]
 コマンド文字列
[戻り値]
 0: コマンド送信後、OKを受信した
 -1: コマンド送信後、OKを受信せずタイムアウトした
*/
int sendCmdAndWaitOk(const char *cmd)
{
  Serial2.println(cmd);
  return waitOk();
}

/* 
チャンネル・スキャン
[引数]
 なし
[戻り値]
 void
[副作用]
 ・para配列に通知されたPAN情報が格納される
*/
void scan()
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
            strcpy(para[BP35C0_CHANNEL_PAGE], &buf[i]);
          else if (strncmp(&buf[2], "Channel", strlen("Channel")) == 0)
            strcpy(para[BP35C0_CHANNEL], &buf[i]);
          else if (strncmp(&buf[2], "Pan ID", strlen("Pan ID")) == 0)
            strcpy(para[BP35C0_PAN_ID], &buf[i]);
          else if (strncmp(&buf[2], "Addr", strlen("Addr")) == 0)
            strcpy(para[BP35C0_ADDR], &buf[i]);
          else if (strncmp(&buf[2], "LQI", strlen("LQI")) == 0)
            strcpy(para[BP35C0_LQI], &buf[i]);
          else if (strncmp(&buf[2], "Side", strlen("Side")) == 0)
            strcpy(para[BP35C0_SIDE], &buf[i]);
          else if (strncmp(&buf[2], "PairID", strlen("PaidID")) == 0)
            strcpy(para[BP35C0_PAIR_ID], &buf[i]);
        }
      }
      vTaskDelay(10 / portTICK_RATE_MS);
    }
  }
  for (int i = 0; i < 7; i++)
  {
    Serial.printf("%d :", i);
    Serial.println(para[i]);
  }
}

/*
スマートメーターへのJOIN
[引数]
 pwd: Bルート・パスワード
 bid: Bルート・ID
[戻り値]
 void
[副作用]
 ・addr配列に、ipv6アドレスが格納される
*/
void join(const char *pwd, const char *bid)
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

    snprintf(buf, sizeof(buf), "SKSREG S2 %s", para[BP35C0_CHANNEL]);
    if (sendCmdAndWaitOk(buf) < 0) // SKSREG S2(Channel)
      continue;
    snprintf(buf, sizeof(buf), "SKSREG S3 %s", para[BP35C0_PAN_ID]);
    if (sendCmdAndWaitOk(buf) < 0) // SKSREG S3(Pan ID)
      continue;
    sendCmdAndWaitIpv6Address(); // SKLL64(Addr)
    // snprintf(buf, sizeof(buf), "SKJOIN %s", addr);
    // if (sendCmdAndWaitOk(buf) < 0) // SKJOIN
    //   continue;

    Serial.println("join end");
    break;
  }
}

// void bp35c0_send_task(void *arm)
// {
//   Serial.println("snd Task start");
//   char buf[128];
//   while (1)
//   {
//     snprintf(buf, sizeof(buf), "SKSENDTO 1 %s 0E1A 1 0 %04d ", addr, sizeof(cmd_buf));
//     Serial.println(buf);
//     // Serial2.println(cmd);
//     // vTaskDelay(60 * 1000 / portTICK_RATE_MS);

//     vTaskDelay(1 * 1000 / portTICK_RATE_MS);
//   }
//   Serial.println("snd Task end");
// }

#define SEND_INTERVAL (60 * 1000 / 1500)
static boolean reqTerm = false;
/*
BP35C0ハンドリング・タスク
*/
void bp35c0_monitoring_task(void *arm)
{
  while (true)
  {
    // スマートメータにJOIN
    Serial.println("term");
    sendCmdAndWaitOk("SKTERM"); // SKTERM
    join(PWD, BID);

    char buf[256];
    int sndCount = 0;
    while (true)
    {
      if (sndCount > 0 && --sndCount == 0)
      {
        // 要求コマンド送信
        sendReqCmd();
        sndCount = SEND_INTERVAL;
      }
      if (reqTerm)
      {
        reqTerm = false;
        Serial.println("term");
        sendCmdAndWaitOk("SKTERM"); // SKTERM
        return;
      }
      size_t len = Serial2.readBytesUntil(0x0a, buf, sizeof(buf));
      if (len > 0)
      {
        buf[len - 1] = 0;
        Serial.println(buf);
        if (strncmp(buf, "ERXUDP", strlen("ERXUDP")) == 0)
        {
        }
        else if (strncmp(buf, "EVENT 25", strlen("EVENT 25")) == 0)
        {
          // 要求コマンドを開始
          sndCount = SEND_INTERVAL;
        }
        else if (strncmp(buf, "EVENT 26", strlen("EVENT 26")) == 0)
        {
          // セッション切断要求 → 再接続開始
          sndCount = 0;
          break;
        }
        else if (strncmp(buf, "EVENT 27", strlen("EVENT 27")) == 0)
        {
          // PANA セッションの終了に成功 → 再接続開始
          sndCount = 0;
          break;
        }
        else if (strncmp(buf, "EVENT 28", strlen("EVENT 28")) == 0)
        {
          // PANA セッションの終了要求に対する応答がなくタイムアウトした(セッションは終了) → 再接続開始
          sndCount = 0;
          break;
        }
        else if (strncmp(buf, "EVENT 29", strlen("EVENT 29")) == 0)
        {
          // セッション期限切れ → 送信を止め、'EVENT 25'を待つ
          sndCount = 0;
        }
      }
      vTaskDelay(500 / portTICK_RATE_MS);
    }
  }
}

void setup()
{
  M5.begin();

  Serial.begin(115200);

  gpio_set_level(BP35C0_RSTn, 0); // BP35C0 Reset = on
  gpio_set_direction(BP35C0_RSTn, GPIO_MODE_OUTPUT);
  delay(100);                     // 100ms wait
  gpio_set_level(BP35C0_RSTn, 1); // BP35C0 Reset = off
  delay(500);                     // 500ms wait

  Serial2.begin(BP35C0_bps, SERIAL_8N1, 16, 17);
  Serial2.setTimeout(1000);
  // Serial2.println("WOPT 01");

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
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
  sendCmdAndWaitOk("SKSREG SFE 0"); // echoなし
}

void SmartMeter::connect(const char *pwd, const char *bid)
{
  char buf[128];
  while (true)
  {
    // Serial.println("join start");
    _debugView->output("join start");

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
    snprintf(buf, sizeof(buf), "SKJOIN %s", _addr);
    if (sendCmdAndWaitOk(buf) < 0) // SKJOIN
      continue;

    _debugView->output("join end");
    break;
  }
}

void SmartMeter::disconnect()
{
  _debugView->output("term");
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
int SmartMeter::request()
{
  _debugView->output("send request");
  char buf[128];
  int len = sizeof(_cmd_buf);
  snprintf(buf, sizeof(buf), "SKSENDTO 1 %s 0E1A 1 0 %04X ", _addr, len);
  Serial2.print(buf);
  for (int i = 0; i < len; i++)
  {
    Serial2.write(_cmd_buf[i]);
  }
  return waitOk();
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
      // _debugView->output(buf);
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
      _debugView->output(_addr);
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
    _debugView->output("wait start");
    delay(10000);
    while (true)
    {
      size_t len = Serial2.readBytesUntil(0x0a, buf, sizeof(buf));
      Serial.printf("%d :", len);
      _debugView->output("wait end");
      if (len > 0)
      {
        buf[len - 1] = 0;
        _debugView->output(buf);
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
    _debugView->output(_para[i]);
  }
}

void SmartMeter::parseE1(u_char *edt)
{
  switch (edt[0])
  {
  case 0x00: // 1kWh
    _k = 1;
    break;
  case 0x01: // 0.1kWh
    _k = 0.1;
    break;
  case 0x02: // 0.01kWh
    _k = 0.01;
    break;
  case 0x03: // 0.001kWh
    _k = 0.001;
    break;
  case 0x04: // 0.0001kWh
    _k = 0.0001;
    break;
  case 0x0a: // 10kWh
    _k = 10.0;
    break;
  case 0x0b: // 100kWh
    _k = 100.0;
    break;
  case 0x0c: // 1000kWh
    _k = 1000.0;
    break;
  case 0x0d: // 10000kWh
    _k = 10000.0;
    break;
  default:
    break;
  }
}

void SmartMeter::parseE7(u_char *edt)
{
  long v = 0;
  for (int i = 0; i < 4; i++)
  {
    v <<= 8;
    v |= edt[i];
  }
  _power = v;
  Serial.printf("%ld[w]", _power);
  Serial.println();
}

#define TIMEZONE 9 * 3600
void SmartMeter::parseEAEB(u_char *edt, time_t *t, float *p)
{
  tm tm;
  tm.tm_year = (edt[0] << 8 | edt[1]) - 1900;
  tm.tm_mon = edt[2] - 1;
  tm.tm_mday = edt[3];
  tm.tm_hour = edt[4];
  tm.tm_min = edt[5];
  tm.tm_sec = edt[6];
  *t = mktime(&tm) - TIMEZONE;
  long v = 0;
  for (int i = 0; i < 4; i++)
  {
    v <<= 8;
    v |= edt[i + 7];
  }
  *p = (float)v * _k;
  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d, %10.2f[kWh]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, *p);
  Serial.println();
}

RCV_CODE SmartMeter::parse(u_char *binBuf, size_t size)
{
  char buf[64];
  RCV_CODE code = ERXUDP;
  for (int j = 0; j < size; j++)
    Serial.printf("<%02x>", binBuf[j]);
  // Serial.printf("buf=%p, len=%d", binBuf, l);
  Serial.println();
  ECHONET_FRAME *ef = (ECHONET_FRAME *)binBuf;
  if (ef->ehd1 == 0x10 && ef->ehd2 == 0x81)
  {
    u_int seoj = ef->seoj[0] << 16 | ef->seoj[1] << 8 | ef->seoj[2];
    u_int deoj = ef->deoj[0] << 16 | ef->deoj[1] << 8 | ef->deoj[2];
    u_short tid = ef->tid[0] << 8 | ef->tid[1];
    // Serial.printf("seoj=%06x, deoj=%06x, tid=%04x, esv=%02x, opc=%02x", seoj, deoj, tid, ef->esv, ef->opc);
    // Serial.println();
    snprintf(buf, sizeof(buf), "seoj=%06x, deoj=%06x, tid=%04x, esv=%02x, opc=%02x", seoj, deoj, tid, ef->esv, ef->opc);
    _debugView->output(buf);
    if (seoj == 0x028801 && deoj == 0x05ff01)
    {
      // 低圧スマート電力量メータ→コントローラ
      ECHONET_DATA *pd = &ef->data;
      for (int i = 0; i < ef->opc; i++)
      {
        int s = pd->pdc;
        // Serial.printf(" epc=%02x, pdc=%d, data=", pd->epc, s);
        // for (int j = 0; j < s; j++)
        //   Serial.printf("<%02x>", pd->edt[j]);
        // Serial.println();
        snprintf(buf, sizeof(buf), " epc=%02x, pdc=%d, data=", pd->epc, s);
        for (int j = 0; j < s; j++)
        {
          int l = strlen(buf);
          snprintf(&buf[l], sizeof(buf) - l, "%02x,", pd->edt[j]);
        }
        _debugView->output(buf);
        // pd->edt parse
        switch (pd->epc)
        {
        case 0xe1: // 積算電力量単位(E1)
          parseE1(pd->edt);
          break;
        case 0xe7: // 瞬時電力計測値(E7)
          parseE7(pd->edt);
          code = ERXUDP_E7;
          break;
        case 0xea: // 正方向定時積算電力量(EA)
          parseEAEB(pd->edt, &_timePlus, &_powerPlus);
          code = ERXUDP_EAB;
          break;
        case 0xeb: // 逆方向定時積算電力量(EA)
          parseEAEB(pd->edt, &_timeMinus, &_powerMinus);
          code = ERXUDP_EAB;
          break;
        }
        pd = (ECHONET_DATA *)(pd->edt + s);
      }
      Serial.println();
    }
  }

  return code;
}

RCV_CODE SmartMeter::polling(char *data, size_t size)
{
  RCV_CODE r = TIMEOUT;
  char buf[1024];
  size_t len = Serial2.readBytesUntil(0x0a, buf, sizeof(buf));
  if (len > 0)
  {
    len--;
    buf[len] = 0;
    // _debugView->output(buf);
    Serial.println(buf);
    if (strncmp(buf, "ERXUDP", strlen("ERXUDP")) == 0)
    {
      // UDP受信
      for (int i = len - 1; i > 0; i--)
      {
        if (buf[i] == 0x20)
        {
          u_char *binBuf;
          size_t l = toBin(&buf[i + 1], &binBuf);
          if (l > 0)
          {
            r = parse(binBuf, l);
            free(binBuf);
          }
          break;
        }
      }
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
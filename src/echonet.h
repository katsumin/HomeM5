#ifndef _ECHONET_H_
#define _ECHONET_H_
#include <Arduino.h>

typedef struct
{
  u_char epc;    // 1 byte
  u_char pdc;    // 1 byte
  u_char edt[1]; // n byte (n > 1)
} ECHONET_DATA;

typedef struct
{
  u_char ehd1;   // 1byte
  u_char ehd2;   // 1byte
  u_char tid[2]; //  2byte
  // EDATA
  u_char seoj[3]; // 3byte
  u_char deoj[3]; // 3byte
  u_char esv;     // 1byte
  u_char opc;     // 1byte
  // 以下、可変長データ
  ECHONET_DATA data;
} ECHONET_FRAME;

#define ASC2BIN(x) (((x > 0x60) ? (x - 0x61 - 0x0a) : ((x > 0x40) ? (x - 0x41 + 0xa) : (x - 0x30))) & 0xf)

class Echonet
{
public:
  size_t toBin(const char *str, u_char **binBuf)
  {
    size_t l = strlen(str) / 2;
    u_char *p = (u_char *)malloc(l);
    if (p == nullptr)
    {
      Serial.println("malloc error");
      return 0;
    }
    for (int i = 0; i < l; i++)
    {
      p[i] = (u_char)(ASC2BIN(str[i * 2]) << 4 | ASC2BIN(str[i * 2 + 1]));
    }
    *binBuf = p;
    return l;
  }
};
#endif
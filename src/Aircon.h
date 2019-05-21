#ifndef _AIRCON_H_
#define _AIRCON_H_
#include <Arduino.h>
#include <Udp.h>
#include "echonet.h"

class Aircon
{
public:
  inline long getPower() { return _power; };
  inline void setPower(long power) { _power = power; };
  inline long getTempRoom() { return _tempRoom; };
  inline void setTempRoom(long temp) { _tempRoom = temp; };
  inline long getTempOut() { return _tempOut; };
  inline void setTempOut(long temp) { _tempOut = temp; };
  void init(EchonetUdp *pUdp, const char *server, void (*callback)(Aircon *))
  {
    IPAddress addr;
    addr.fromString(server);
    _server = addr;
    _pEchonetUdp = pUdp;
    _pEchonetUdp->setCallback(addr, &parse, this);
    _callback = callback;
  };
  void request()
  {
    Serial.print("request -> ");
    Serial.println(_server);
    UDP *udp = _pEchonetUdp->getUdp();
    udp->beginPacket(_server, 3610); // Echonet requests are to port 3610
    udp->write(_cmd_buf, sizeof(_cmd_buf));
    udp->endPacket();
  };

private:
  long _power;
  long _tempRoom;
  long _tempOut;
  IPAddress _server;
  EchonetUdp *_pEchonetUdp = nullptr;
  void (*_callback)(Aircon *) = nullptr;
  u_char _cmd_buf[18] = {
      0x10,
      0x81, // EHD
      0x00,
      0x01, // TID
      0x05,
      0xff,
      0x01, // SEOJ
      0x01,
      0x30,
      0x01, // DEOJ
      0x62, // ESV(プロパティ値読み出し要求)
      0x03, // OPC(3data)
      0x84, // EPC()
      0x00, // PDC
      0xbe, // EPC()
      0x00, // PDC
      0xbb, // EPC()
      0x00, // PDC
  };
  static void parse(ECHONET_FRAME *ef, void *obj)
  {
    Aircon *ac = (Aircon *)obj;
    ECHONET_DATA *pd = &ef->data;
    for (int i = 0; i < ef->opc; i++)
    {
      uint16_t d = 0;
      int s = pd->pdc;
      switch (pd->epc)
      {
      case 0x84: // 消費電力
        for (int j = 0; j < s; j++)
        {
          d <<= 8;
          d |= pd->edt[j];
        }
        ac->setPower(d);
        break;
      case 0xbe: // 外気温
        ac->setTempOut(pd->edt[0]);
        break;
      case 0xbb: // 室温
        ac->setTempRoom(pd->edt[0]);
        break;
      }
      pd = (ECHONET_DATA *)(pd->edt + s);
    }
    ac->_callback(ac);
  };
};

Aircon aircon;
#endif
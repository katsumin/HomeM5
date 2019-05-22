#ifndef _ECOCUTE_H_
#define _ECOCUTE_H_
#include <Arduino.h>
#include <Udp.h>
#include "echonet.h"

// const int PACKET_SIZE = 256; // NTP time stamp is in the first 48 bytes of the message

enum ECOCUTE_STATE
{
  // Idle
  IDLE,
  // Requested
  REQUESTED,
  // Updated
  UPDATED,
};

class Ecocute
{
public:
  // void update() { xTaskCreate(&Ecocute::_task, "EcocuteTask", 4096, this, 5, NULL); };
  inline void setPower(long power) { _power = power; };
  inline long getPower() { return _power; };
  inline void setTotalPower(long power) { _totalPower = power; };
  inline long getTotalPower() { return _totalPower; };
  inline void setTank(long tank) { _tank = tank; };
  inline long getTank() { return _tank; };
  inline void setState(ECOCUTE_STATE state) { _state = state; };
  inline ECOCUTE_STATE getState() { return _state; };
  void init(EchonetUdp *pUdp, const char *server, void (*callback)(Ecocute *))
  {
    IPAddress addr;
    addr.fromString(server);
    _server = addr;
    _pEchonetUdp = pUdp;
    _pEchonetUdp->setCallback(addr, &parse, this);
    _callback = callback;
    setState(IDLE);
  }
  void request()
  {
    setState(REQUESTED);
    Serial.print("request -> ");
    Serial.println(_server);
    UDP *udp = _pEchonetUdp->getUdp();
    udp->beginPacket(_server, 3610); // Echonet requests are to port 3610
    udp->write(_cmd_buf, sizeof(_cmd_buf));
    udp->endPacket();
  };

private:
  long _totalPower;
  long _power;
  long _tank;
  ECOCUTE_STATE _state;
  IPAddress _server;
  EchonetUdp *_pEchonetUdp = nullptr;
  void (*_callback)(Ecocute *) = nullptr;
  u_char _cmd_buf[18] = {
      0x10,
      0x81, // EHD
      0x00,
      0x01, // TID
      0x05,
      0xff,
      0x01, // SEOJ
      0x02,
      0x6b,
      0x01, // DEOJ
      0x62, // ESV(プロパティ値読み出し要求)
      0x03, // OPC(3data)
      0x84, // EPC()
      0x00, // PDC
      0x85, // EPC()
      0x00, // PDC
      0xe1, // EPC()
      0x00, // PDC
  };

private:
  static void parse(ECHONET_FRAME *ef, void *obj)
  {
    Ecocute *ecocute = (Ecocute *)obj;
    ECHONET_DATA *pd = &ef->data;
    for (int i = 0; i < ef->opc; i++)
    {
      long d = 0;
      int s = pd->pdc;
      for (int j = 0; j < s; j++)
      {
        d <<= 8;
        d |= pd->edt[j];
      }
      switch (pd->epc)
      {
      case 0x84: // 消費電力
        ecocute->setPower(d);
        break;
      case 0x85: // 積算消費電力
        ecocute->setTotalPower(d);
        break;
      case 0xe1: // タンク残湯量
        ecocute->setTank(d);
        break;
      }
      pd = (ECHONET_DATA *)(pd->edt + s);
    }
    ecocute->_callback(ecocute);
    ecocute->setState(UPDATED);
  }
};

Ecocute ecocute;
#endif
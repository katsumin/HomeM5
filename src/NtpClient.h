#ifndef _NTPCLIENT_H_
#define _NTPCLIENT_H_
#include <Arduino.h>
#include <Udp.h>

const char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48;            // NTP time stamp is in the first 48 bytes of the message

class NtpClient
{
public:
  void update()
  {
    xTaskCreate(&NtpClient::_task, "Ntp", 4096, this, 5, NULL);
  };
  void init(UDP *pUdp, char *server, unsigned int port = 8888)
  {
    _pUdp = pUdp;
    _server = server;
    if (_pUdp != nullptr)
    {
      _localPort = port;
      _pUdp->begin(_localPort);
      Serial.print(" bind port: ");
      Serial.println(_localPort);
    }
  }
  inline unsigned long getMillis() { return _getMillis; };
  inline time_t getEpocTime() { return _epochTime; };

private:
  char *_server;
  UDP *_pUdp = nullptr;
  unsigned int _localPort = 8888;         // local port to listen for UDP packets
  uint8_t _packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
  unsigned long _getMillis = 0;
  time_t _epochTime;

private:
  void static _task(void *arm)
  {
    NtpClient *ntp = (NtpClient *)arm;
    ntp->_getMillis = 0;
    Serial.println("ntp start");
    UDP *pUdp = ntp->_pUdp;
    while (true)
    {
      ntp->_sendNTPpacket();
      int i = 0;
      for (; i < 10; i++)
      {
        Serial.println("waiting recieve");
        delay(1000);
        if (pUdp->parsePacket() > 0)
        {
          uint8_t buf[NTP_PACKET_SIZE];     //buffer to hold incoming and outgoing packets
          pUdp->read(buf, NTP_PACKET_SIZE); // read the packet into the buffer
          ntp->_getMillis = millis();
          Serial.println("recieved");
          unsigned long highWord = word(buf[40], buf[41]);
          unsigned long lowWord = word(buf[42], buf[43]);
          // combine the four bytes (two words) into a long integer
          // this is NTP time (seconds since Jan 1 1900):
          unsigned long secsSince1900 = highWord << 16 | lowWord;
          // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
          const unsigned long seventyYears = 2208988800UL;
          // subtract seventy years:
          ntp->_epochTime = secsSince1900 - seventyYears;
          break;
        }
      }
      if (i < 10)
        break;
    }
    Serial.println("ntp stop");
    vTaskDelete(NULL);
  }
  void _sendNTPpacket()
  {
    if (_pUdp == nullptr)
      return;
    // set all bytes in the buffer to 0
    memset(_packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    _packetBuffer[0] = 0b11100011; // LI, Version, Mode
    _packetBuffer[1] = 0;          // Stratum, or type of clock
    _packetBuffer[2] = 6;          // Polling Interval
    _packetBuffer[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    _packetBuffer[12] = 49;
    _packetBuffer[13] = 0x4E;
    _packetBuffer[14] = 49;
    _packetBuffer[15] = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    _pUdp->beginPacket(_server, 123); // NTP requests are to port 123
    _pUdp->write(_packetBuffer, NTP_PACKET_SIZE);
    _pUdp->endPacket();
  }
};
#endif
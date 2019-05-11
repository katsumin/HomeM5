#ifndef _INFLUX_DB_H_
#define _INFLUX_DB_H_
#include <Client.h>
#include <ArduinoHttpClient.h>
#include <Ethernet.h>
#include <WiFiClient.h>

class InfluxDb
{
public:
  InfluxDb(const char *server, const char *db, int port = 8086)
  {
    _server = String(server);
    _port = port;
    _db = String(db);
    _mutex = xSemaphoreCreateMutex();
  }
  // void setClient(Client *client) { _pClient = client; }
  int write(char *data);
  inline void setEthernet(boolean ether) { _isEthernet = ether; }

private:
  WiFiClient _wc;
  EthernetClient _ec;
  boolean _isEthernet;
  // Client *_pClient;
  String _server;
  int _port;
  String _db;
  xSemaphoreHandle _mutex;
};

#endif

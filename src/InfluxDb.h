#ifndef _INFLUX_DB_H_
#define _INFLUX_DB_H_
#include <Client.h>
#include <ArduinoHttpClient.h>

class InfluxDb
{
public:
  InfluxDb(const char *server, const char *db, int port = 8086)
  {
    _server = String(server);
    _port = port;
    _db = String(db);
  }
  void setClient(Client *client) { _pClient = client; }
  int write(char *data);

private:
  Client *_pClient;
  String _server;
  int _port;
  String _db;
};

#endif

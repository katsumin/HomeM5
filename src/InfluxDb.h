#ifndef _INFLUX_DB_H_
#define _INFLUX_DB_H_
#include <Client.h>
#include <ArduinoHttpClient.h>
#include <Ethernet3.h>
#include <WiFiClient.h>

class InfluxDb
{
public:
    InfluxDb(const char *server, const char *db, int port = 8086);
    int write(const char *data);
    inline void setEthernet(boolean ether) { _isEthernet = ether; }
    void init(Client *pC);

private:
    HttpClient *_http;
    boolean _isEthernet;
    String _server;
    int _port;
    String _db;
    char *_buf;
};

#endif

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
    // {
    //     _server = String(server);
    //     _port = port;
    //     _db = String(db);
    //     _mutex = xSemaphoreCreateMutex();
    // }
    int write(const char *data);
    inline void setEthernet(boolean ether) { _isEthernet = ether; }
    void init(Client *pC);

private:
    HttpClient *_http;
    // WiFiClient _wc;
    // EthernetClient _ec;
    boolean _isEthernet;
    String _server;
    int _port;
    String _db;
    // xSemaphoreHandle _mutex;
    char *_buf;
};

#endif

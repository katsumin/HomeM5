#include "InfluxDb.h"

// #define TEST
#define RETRY (1)
#define BUFSIZE (256)
InfluxDb::InfluxDb(const char *server, const char *db, int port)
{
    _server = String(server);
    _port = port;
    _db = String(db);
    // _mutex = xSemaphoreCreateMutex();
}
void InfluxDb::init(Client *pC)
{
    _http = new HttpClient(*pC, _server, _port);
    _buf = new char[BUFSIZE];
}
int InfluxDb::write(const char *data)
{
#ifdef TEST
    return 204;
#endif
    // xSemaphoreTake(_mutex, portMAX_DELAY);
    int res = 0;
    // Client *pC;
    // if (_isEthernet)
    // {
    //     // Serial.printf("Ether: %d", _ec.availableForWrite());
    //     // Serial.println();
    //     pC = &_ec;
    // }
    // else
    // {
    //     pC = &_wc;
    // }
    // char buf[1024];
    snprintf(_buf, BUFSIZE, "/write?db=%s", _db.c_str());
#ifdef DEBUG
    Serial.println(data);
#endif
    // HttpClient http(*pC, _server, _port);
    // http.post(buf, "text/plain", data);
    // res = http.responseStatusCode();
    // Serial.println(data);
    _http->post(_buf, "text/plain", data);
    Serial.println("posted.");
    res = _http->responseStatusCode();
    Serial.printf("response: %d", res);
    Serial.println();
    // String rbody = http.responseBody();
    String rbody = _http->responseBody();
    Serial.printf("response body: %s", rbody.c_str());
    Serial.println();
    // xSemaphoreGive(_mutex);
    // http.stop();
    _http->stop();
    return res;
}
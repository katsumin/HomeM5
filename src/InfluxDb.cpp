#include "InfluxDb.h"

// #define TEST
#define RETRY (1)
#define BUFSIZE (256)
InfluxDb::InfluxDb(const char *server, const char *db, int port)
{
    _server = String(server);
    _port = port;
    _db = String(db);
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
    int res = 0;
    snprintf(_buf, BUFSIZE, "/write?db=%s", _db.c_str());
#ifdef DEBUG
    Serial.println(data);
#endif
    _http->post(_buf, "text/plain", data);
    Serial.println("posted.");
    res = _http->responseStatusCode();
    Serial.printf("response: %d", res);
    Serial.println();
    String rbody = _http->responseBody();
    Serial.printf("response body: %s", rbody.c_str());
    Serial.println();
    _http->stop();
    return res;
}
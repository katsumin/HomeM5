#include "InfluxDb.h"

int InfluxDb::write(char *data)
{
  HttpClient http(*_pClient, _server, _port);
  char buf[256];
  snprintf(buf, sizeof(buf), "/write?db=%s", _db.c_str());
  http.post(buf, "text/plain", data);
  return http.responseStatusCode();
}
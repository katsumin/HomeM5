#include "InfluxDb.h"
#include <Ethernet.h>

int InfluxDb::write(char *data)
{
  xSemaphoreTake(_mutex, portMAX_DELAY);
  Client *pC;
  if (_isEthernet)
  {
    Serial.printf("Ether: %d", _ec.availableForWrite());
    Serial.println();
    pC = &_ec;
  }
  else
  {
    pC = &_wc;
  }
  char buf[256];
  snprintf(buf, sizeof(buf), "/write?db=%s", _db.c_str());
  HttpClient http(*pC, _server, _port);
  http.post(buf, "text/plain", data);
  int res = http.responseStatusCode();
  xSemaphoreGive(_mutex);
  return res;
}
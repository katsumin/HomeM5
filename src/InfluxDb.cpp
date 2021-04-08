#include "InfluxDb.h"
#include <Ethernet3.h>

// #define TEST
#define RETRY (1)
int InfluxDb::write(const char *data)
{
#ifdef TEST
    return 204;
#endif
    xSemaphoreTake(_mutex, portMAX_DELAY);
    int res = 0;
    // for (int i = 0; i < RETRY; i++)
    // {
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
#ifdef DEBUG
    Serial.println(data);
#endif
    HttpClient http(*pC, _server, _port);
    http.post(buf, "text/plain", data);
    res = http.responseStatusCode();
    // Serial.println(res);
    // if (res == 204)
    //     break;
    // Serial.println("retry ");
    // delay(10);
    // }
    xSemaphoreGive(_mutex);
    return res;
}
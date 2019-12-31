#ifndef _SMARTMETER_H_
#define _SMARTMETER_H_

#include <Arduino.h>
#include <Udp.h>
#include "DataStore.h"
#include "EchonetUdp.h"
#define TIMEZONE 9 * 3600

class SmartMeter
{
public:
    inline long getPower() { return _power; };
    inline void setPower(long power) { _power = power; };
    inline float getWattHourPlus() { return _powerPlus; };
    inline void setWattHourPlus(float powerPlus) { _powerPlus = powerPlus; }
    inline float getWattHourMinus() { return _powerMinus; };
    inline void setWattHourMinus(float powerMinus) { _powerMinus = powerMinus; }
    inline time_t getTimePlus() { return _timePlus; };
    inline void setTimePlus(time_t timePlus) { _timePlus = timePlus; }
    inline time_t getTimeMinus() { return _timeMinus; };
    inline void setTimeMinus(time_t timeMinus) { _timeMinus = timeMinus; }

    void init(EchonetUdp *pUdp, const char *server, void (*callback)(SmartMeter *))
    {
        _server.fromString(server);
        _pEchonetUdp = pUdp;
        _pEchonetUdp->setCallback(_server, &parse, this);
        _callback = callback;
        if (_pEchonetUdp->getEthernet())
            _pSendUdp = new EthernetUDP();
        else
            _pSendUdp = new WiFiUDP();
    };
    void request()
    {
        Serial.print("request -> ");
        Serial.println(_server);
        UDP *udp = _pEchonetUdp->getUdp();
        udp->beginPacket(_server, 3610); // Echonet requests are to port 3610
        udp->write(_cmd_buf, sizeof(_cmd_buf));
        udp->endPacket();
        // _pSendUdp->beginPacket(_server, 3610); // Echonet requests are to port 3610
        // _pSendUdp->write(_cmd_buf, sizeof(_cmd_buf));
        // _pSendUdp->endPacket();
    };
    inline void setDataStore(DataStore *store) { _dataStore = store; };

private:
    UDP *_pSendUdp;
    DataStore *_dataStore;
    float _k = 0.1;
    long _power;
    float _powerPlus;
    float _powerMinus;
    time_t _timePlus;
    time_t _timeMinus;
    IPAddress _server;
    EchonetUdp *_pEchonetUdp = nullptr;
    void (*_callback)(SmartMeter *) = nullptr;
    u_char _cmd_buf[20] = {
        0x10,
        0x81, // EHD
        0x00,
        0x01, // TID
        0x05,
        0xff,
        0x01, // SEOJ
        0x02,
        0x88,
        0x01, // DEOJ
        0x62, // ESV(プロパティ値読み出し要求)
        0x04, // OPC(4data)
        0xe7, // EPC()
        0x00, // PDC
        0xe1, // EPC()
        0x00, // PDC
        0xea, // EPC()
        0x00, // PDC
        0xeb, // EPC()
        0x00, // PDC
    };
    void parseE1(u_char *edt)
    {
        switch (edt[0])
        {
        case 0x00: // 1kWh
            _k = 1;
            break;
        case 0x01: // 0.1kWh
            _k = 0.1;
            break;
        case 0x02: // 0.01kWh
            _k = 0.01;
            break;
        case 0x03: // 0.001kWh
            _k = 0.001;
            break;
        case 0x04: // 0.0001kWh
            _k = 0.0001;
            break;
        case 0x0a: // 10kWh
            _k = 10.0;
            break;
        case 0x0b: // 100kWh
            _k = 100.0;
            break;
        case 0x0c: // 1000kWh
            _k = 1000.0;
            break;
        case 0x0d: // 10000kWh
            _k = 10000.0;
            break;
        default:
            break;
        }
    }
    void parseE7(u_char *edt)
    {
        long v = 0;
        for (int i = 0; i < 4; i++)
        {
            v <<= 8;
            v |= edt[i];
        }
        setPower(v);
        Serial.printf("%ld[w]", _power);
        Serial.println();
    }
    void parseEAEB(u_char *edt, time_t *t, float *p)
    {
        tm tm;
        tm.tm_year = (edt[0] << 8 | edt[1]) - 1900;
        tm.tm_mon = edt[2] - 1;
        tm.tm_mday = edt[3];
        tm.tm_hour = edt[4];
        tm.tm_min = edt[5];
        tm.tm_sec = edt[6];
        *t = mktime(&tm);
        long v = 0;
        for (int i = 0; i < 4; i++)
        {
            v <<= 8;
            v |= edt[i + 7];
        }
        *p = (float)v * _k;
        Serial.printf("%04d/%02d/%02d %02d:%02d:%02d, %10.2f[kWh]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, *p);
        Serial.println();
    }
    static void parse(ECHONET_FRAME *ef, void *obj)
    {
        SmartMeter *sm = (SmartMeter *)obj;
        ECHONET_DATA *pd = &ef->data;
        for (int i = 0; i < ef->opc; i++)
        {
            time_t t;
            float p;
            // uint16_t d = 0;
            int s = pd->pdc;
            switch (pd->epc)
            {
            case 0xe7: // 瞬時電力
                sm->parseE7(pd->edt);
                sm->_dataStore->setPower(sm->getPower());
                break;
            case 0xea: // 積算電力量（正方向）
                sm->parseEAEB(pd->edt, &t, &p);
                sm->setWattHourPlus(p);
                sm->setTimePlus(t);
                sm->_dataStore->setWattHourPlus(p, t);
                break;
            case 0xeb: // 積算電力量（逆方向）
                sm->parseEAEB(pd->edt, &t, &p);
                sm->setWattHourMinus(p);
                sm->setTimeMinus(t);
                sm->_dataStore->setWattHourMinus(p, t);
                break;
            }
            pd = (ECHONET_DATA *)(pd->edt + s);
        }
        sm->_callback(sm);
    };
};

SmartMeter smartmeter;
#endif
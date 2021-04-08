#ifndef _ETHER_MANAGER_H_
#define _ETHER_MANAGER_H_
#include "EL.h"
#include "DataStore.h"

// ユニキャスト用ソケットを使うようにELクラスを拡張
class EL2 : public EL
{
private:
    UDP *_uniUdp = nullptr;
    uint16_t _uniPort = EL_PORT;

public:
    EL2(UDP &udp, byte eoj0, byte eoj1, byte eoj2) : EL(udp, eoj0, eoj1, eoj2)
    {
        _uniUdp = &udp;
    }
    EL2(UDP &udp, byte eojs[][3], int count) : EL(udp, eojs, count)
    {
        _uniUdp = &udp;
    }
    EL2(UDP &udp, UDP &uniUdp, byte eoj0, byte eoj1, byte eoj2) : EL(udp, eoj0, eoj1, eoj2)
    {
        _uniUdp = &uniUdp;
        _uniPort = EL_PORT + 1;
    }
    EL2(UDP &udp, UDP &uniUdp, byte eojs[][3], int count) : EL(udp, eojs, count)
    {
        _uniUdp = &uniUdp;
        _uniPort = EL_PORT + 1;
    }

    void begin(void)
    {
        Serial.println("EL2::begin");
        // udp
        if (_uniUdp->begin(_uniPort))
        {
            Serial.println("EL.udp.begin successful.");
        }
        else
        {
            Serial.println("Reseiver udp.begin failed."); // localPort
        }

        if (_udp->beginMulticast(_multi, EL_PORT))
        {
            Serial.println("EL.udp.beginMulticast successful.");
        }
        else
        {
            Serial.println("Reseiver EL.udp.beginMulticast failed."); // localPort
        }

        // profile object
        profile[0x80] = new byte[1 + 1]{1, 0x30};                                                                                                   // power
        profile[0x81] = new byte[1 + 1]{1, 0x00};                                                                                                   // position
        profile[0x82] = new byte[4 + 1]{4, 0x01, 0x0a, 0x01, 0x00};                                                                                 // Ver 1.10 (type 1)
        profile[0x83] = new byte[17 + 1]{17, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // identification number
        profile[0x88] = new byte[1 + 1]{1, 0x42};                                                                                                   // error status
        profile[0x8a] = new byte[3 + 1]{3, 0x00, 0x00, 0x00};                                                                                       // maker other
        profile[0x9d] = new byte[2 + 1]{2, 0x01, 0x80};                                                                                             // inf property map
        profile[0x9e] = new byte[2 + 1]{2, 0x01, 0x80};                                                                                             // set property map
        profile[0x9f] = new byte[15 + 1]{15, 0x0e, 0x80, 0x81, 0x82, 0x83, 0x88, 0x8a, 0x9d, 0x9e, 0x9f, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7};             // get property map
        profile[0xd3] = new byte[3 + 1]{3, 0x00, 0x00, byte(deviceCount)};                                                                          // total instance number
        profile[0xd4] = new byte[2 + 1]{2, 0x00, byte(deviceCount + 1)};                                                                            // total class number
        profile[0xd5] = new byte[2 + deviceCount * sizeof(byte[3])]{byte(1 + deviceCount * 3), byte(deviceCount)};                                  // obj list
        profile[0xd6] = new byte[2 + deviceCount * sizeof(byte[3])]{byte(1 + deviceCount * 3), byte(deviceCount)};                                  // obj list
        profile[0xd7] = new byte[2 + deviceCount * sizeof(byte[2])]{byte(1 + deviceCount * 2), byte(deviceCount)};                                  // class list
        for (int i = 0; i < deviceCount; i++)
        {
            memcpy(&profile[0xd5][2 + i * sizeof(byte[3])], &_eojs[i * sizeof(byte[3])], sizeof(byte[3]));
            memcpy(&profile[0xd6][2 + i * sizeof(byte[3])], &_eojs[i * sizeof(byte[3])], sizeof(byte[3]));
            memcpy(&profile[0xd7][2 + i * sizeof(byte[2])], &_eojs[i * sizeof(byte[3])], sizeof(byte[2]));
        }

        // device object
        details[0x80] = profile[0x80];                                                                    // power
        details[0x81] = profile[0x81];                                                                    // position
        details[0x82] = profile[0x82];                                                                    // release K
        details[0x83] = profile[0x83];                                                                    // identification number
        details[0x88] = profile[0x88];                                                                    // error status
        details[0x8a] = profile[0x8a];                                                                    // maker other
        details[0x9d] = new byte[1 + 1]{1, 0x00};                                                         // inf property map
        details[0x9e] = new byte[1 + 1]{1, 0x00};                                                         // set property map
        details[0x9f] = new byte[10 + 1]{10, 0x09, 0x80, 0x81, 0x82, 0x83, 0x88, 0x8a, 0x9d, 0x9e, 0x9f}; // get property map

        details.printAll();

        const byte seoj[] = {0x0e, 0xf0, 0x01};
        const byte deoj[] = {0x0e, 0xf0, 0x01};
        sendMultiOPC1(seoj, deoj, EL_INF, 0xd5, profile[0xd5]);
    }

    // IP指定による送信
    void send(IPAddress toip, byte sBuffer[], int size)
    {
        // Serial.println("EL2::send");
#ifdef EL_DEBUG
        Serial.print("send packet: ");
        for (int i = 0; i < size; i += 1)
        {
            Serial.print(sBuffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println(".");
#endif
        if (_uniUdp->beginPacket(toip, EL_PORT))
        {
            // Serial.println("UDP beginPacket Successful.");
            _uniUdp->write(sBuffer, size);
        }
        else
        {
            Serial.println("UDP beginPacket failed.");
        }

        if (_uniUdp->endPacket())
        {
            // Serial.println("UDP endPacket Successful.");
        }
        else
        {
            Serial.println("UDP endPacket failed.");
        }
    }
};

class EthernetManager
{
private:
    EL *_echo;
    DataStore *_dataStore;

public:
    EthernetManager(UDP *udp, UDP *uniUdp)
    {
        if (uniUdp != nullptr)
        {
            _echo = new EL2(*udp, *uniUdp, 0x05, 0xff, 0x01);
        }
        else
        {
            _echo = new EL2(*udp, 0x05, 0xff, 0x01);
        }
        _echo->begin();
    }
    ~EthernetManager();
    inline void setDataStore(DataStore *store)
    {
        _dataStore = store;
        _dataStore->setEchonet(_echo);
    }
    void processingProperty(const byte *props, IPAddress addr, const byte *seoj)
    {
        const byte epc = props[EL_EPC - EL_EPC];
        const byte pdc = props[EL_PDC - EL_EPC];
#ifdef DEBUG
        char buf[64];
        snprintf(buf, sizeof(buf), "  %02x, %d, ", epc, pdc);
        Serial.print(buf);
        const byte *edt = &props[EL_EDT - EL_EPC];
        for (int j = 0; j < pdc; j++)
        {
            Serial.printf("%02x", edt[j]);
        }
        Serial.println();
#endif
        switch (epc)
        {
        case 0xd6:
            _echo->sendOPC1(addr, seoj, EL_GET, 0x83, {0x00});
            break;
        }
        _dataStore->processingProperty(props, addr, seoj);
    }
    void request()
    {
        _dataStore->request();
    }
    void update()
    {
        int packetSize = 0;
        if (0 != (packetSize = _echo->read()))
        {
            char buf[64];
            IPAddress addr = _echo->remoteIP();
            const byte seoj[] = {_echo->_rBuffer[EL_SEOJ], _echo->_rBuffer[EL_SEOJ + 1], _echo->_rBuffer[EL_SEOJ + 2]};
            snprintf(buf, sizeof(buf), " %s: %02x%02x%02x, ", addr.toString().c_str(), seoj[0], seoj[1], seoj[2]);
#ifdef DEBUG
            Serial.printf("packetSize:%d", packetSize);
            Serial.println();
            Serial.print(buf);
            Serial.println();
#endif
            if (packetSize < EL_EDT)
            {
                Serial.println("size error !");
                return;
            }

            switch (_echo->_rBuffer[EL_ESV])
            {
            case EL_GET:
                _echo->returner();
                break;
            case EL_GET_RES:
            case EL_INF:
            {
                const byte opc = _echo->_rBuffer[EL_OPC];
                const byte *props = &(_echo->_rBuffer[EL_EPC]);
                for (int i = 0; i < opc; i++)
                {
                    processingProperty(props, addr, seoj);
                    const byte pdc = props[EL_PDC - EL_EPC];
                    props += pdc + 1 + 1;
                }
            }
            default:
                break;
            }
        }
    }
    void scan()
    {
        const byte deoj[] = {0x0e, 0xf0, 0x01};
        const byte pdc[] = {0x00};
        _echo->sendMultiOPC1(deoj, EL_GET, 0xd6, pdc);
    }
};

#endif
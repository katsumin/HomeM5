#ifndef _ECHONETUDP_H_
#define _ECHONETUDP_H_
#include <WiFi.h>
#include <Ethernet.h>
#include <map>
#include "echonet.h"

const int PACKET_SIZE = 256; // NTP time stamp is in the first 48 bytes of the message

class EchonetUdp : public Echonet
{
public:
    virtual void init(UDP *pUdp, unsigned int port = 3610)
    {
        _pUdp = pUdp;
        _localPort = port;
        _active = true;
        if (pUdp != nullptr)
            xTaskCreate(&EchonetUdp::_task, "EconetTask", 4096, this, 5, NULL);
    };
    inline virtual void setCallback(IPAddress key, void (*p)(ECHONET_FRAME *ef, void *obj), void *arg)
    {
        _callbacks[key] = p;
        _callbackObjs[key] = arg;
    };
    inline virtual void stop() { _active = false; };
    inline virtual UDP *getUdp() { return _pUdp; };
    inline void setEthernet(boolean ether) { _isEthernet = ether; }
    inline boolean getEthernet() { return _isEthernet; }
    char *getServer()
    {
        UDP *udp = NULL;
        if (getEthernet())
            udp = new EthernetUDP();
        else
            udp = new WiFiUDP();
        udp->beginPacket("224.0.23.0", 3610); // Echonet requests are to port 3610
        udp->write(_cmd_buf, sizeof(_cmd_buf));
        udp->endPacket();
    }

private:
    boolean _isEthernet;
    boolean _active = false;
    UDP *_pUdp = nullptr;
    unsigned int _localPort;
    std::map<IPAddress, void (*)(ECHONET_FRAME *ef, void *obj)> _callbacks;
    std::map<IPAddress, void *> _callbackObjs;
    u_char _cmd_buf[14] = {
        0x10,
        0x81, // EHD
        0x00,
        0x01, // TID
        0x0e,
        0xf0,
        0x01, // SEOJ
        0x0e,
        0xf0,
        0x00, // DEOJ
        0x62, // ESV(プロパティ値読み出し要求)
        0x01, // OPC(1data)
        0xd6, // EPC()
        0x00, // PDC
    };

private:
    void static _task(void *arm)
    {
        Serial.println("EchonetUdp task start");
        EchonetUdp *own = (EchonetUdp *)arm;
        UDP *pUdp = own->_pUdp;
        // IPAddress multicast_addr;
        // multicast_addr.fromString("224.0.23.0");
        uint8_t r = pUdp->begin(own->_localPort);
        // uint8_t r = 0;
        // if (own->_isEthernet)
        //     r = ((EthernetUDP *)pUdp)->beginMulticast(IPAddress(224, 0, 23, 0), own->_localPort);
        // else
        //     r = ((WiFiUDP *)pUdp)->beginMulticast(IPAddress(224, 0, 23, 0), own->_localPort);
        if (r == 1)
        {
            Serial.print(" bind port: ");
            Serial.println(own->_localPort);
            uint8_t binBuf[PACKET_SIZE]; //buffer to hold incoming and outgoing packets
            while (own->_active)
            {
                int rcv_size = pUdp->parsePacket();
                if (rcv_size > 0)
                {
                    IPAddress addr = pUdp->remoteIP();
                    pUdp->read(binBuf, rcv_size); // read the packet into the buffer
                    Serial.print(pUdp->remoteIP());
                    Serial.print(" -> ");
                    Serial.print(rcv_size);
                    Serial.println(" bytes recieved");
                    ECHONET_FRAME *ef = (ECHONET_FRAME *)binBuf;
                    if (ef->ehd1 == 0x10 && ef->ehd2 == 0x81)
                    {
                        // debug dump
                        own->_dump(ef);
                        // parse
                        if (own->_callbacks.count(addr))
                            own->_callbacks[addr](ef, own->_callbackObjs[addr]);
                    }
                }
                delay(1000);
            }
            Serial.println("EchonetUdp task end");
            pUdp->stop();
            vTaskDelete(NULL);
        };
    };

    void _dump(ECHONET_FRAME *ef)
    {
        char buf[64];
        u_int seoj = ef->seoj[0] << 16 | ef->seoj[1] << 8 | ef->seoj[2];
        u_int deoj = ef->deoj[0] << 16 | ef->deoj[1] << 8 | ef->deoj[2];
        u_short tid = ef->tid[0] << 8 | ef->tid[1];
        snprintf(buf, sizeof(buf), "seoj=%06x, deoj=%06x, tid=%04x, esv=%02x, opc=%02x", seoj, deoj, tid, ef->esv, ef->opc);
        Serial.println(buf);
        // if (seoj == 0x026b01 && deoj == 0x05ff01)
        // {
        // エコキュート→コントローラ
        ECHONET_DATA *pd = &ef->data;
        for (int i = 0; i < ef->opc; i++)
        {
            int s = pd->pdc;
            snprintf(buf, sizeof(buf), " epc=%02x, pdc=%d, data=", pd->epc, s);
            for (int j = 0; j < s; j++)
            {
                int l = strlen(buf);
                snprintf(&buf[l], sizeof(buf) - l, "%02x,", pd->edt[j]);
            }
            pd = (ECHONET_DATA *)(pd->edt + s);
            Serial.println(buf);
        }
        // }
    };
};

EchonetUdp echonetUdp;
#endif
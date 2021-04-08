#include "Node.h"
#include "DataStore.h"

Node::Node(IPAddress addr, DataStore *ds)
{
    _addr = addr;
    _id = addr.toString();
    _dataStore = ds;
}

void Node::deviceRequest(Device *device)
{
    byte *data = device->request();
    int len = data[0];
    _echo->send(_addr, &data[1], len);
#ifdef DEBUG
    Serial.printf("request packet(%s): ", _addr.toString().c_str());
    for (int i = 1; i <= len; i++)
        Serial.printf("%02x ", data[i]);
    Serial.println(".");
#endif
}

void Node::request()
{
    for (auto itr = _devices.begin(); itr != _devices.end(); itr++)
    {
        deviceRequest(itr->second);
    }
}
void Node::updateInflux(unsigned long t)
{
    for (auto itr = _devices.begin(); itr != _devices.end(); ++itr)
    {
        uint32_t key = itr->first;
        char **statements = itr->second->getInfluxStatement(getId(), t);
        if (statements != nullptr)
        {
            for (int i = 0;; i++)
            {
                char *statement = statements[i];
                if (statement == nullptr)
                    break;
#ifdef DEBUG
                Serial.printf("influx: %s", statement);
                Serial.println();
#endif
                _dataStore->getInfluxdb()->write(statement);
            }
        }
    }
}

void Node::parse(const byte *props, const byte *seoj)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    uint16_t ct = classType(seoj[0], seoj[1]);
    if (ct == 0x0ef0)
    {
        switch (ed->epc)
        {
        case 0x83:
            // 識別番号
            {
                int l = ed->pdc;
                _id = hexString(ed->edt, l);
            }
            break;
        case 0xd5:
        case 0xd6:
            // インスタンスリスト
            {
                int c = ed->edt[0];
                for (int i = 0; i < c; i++)
                {
                    uint32_t key = deviceId(ed->edt[i * 3 + 1], ed->edt[i * 3 + 2], ed->edt[i * 3 + 3]);
                    Serial.printf("   %02x%02x%02x", ed->edt[i * 3 + 1], ed->edt[i * 3 + 2], ed->edt[i * 3 + 3]);
                    if (_devices.count(key) == 0)
                    {
                        Device *d = Device::createInstance(ed->edt[i * 3 + 1], ed->edt[i * 3 + 2], ed->edt[i * 3 + 3], this);
                        if (d != nullptr)
                        {
                            Serial.print(" ->created.");
                            _devices[key] = d;
                            ViewController *vc = _dataStore->getViewController();
                            DeviceView *dv = DeviceView::createView(d, vc->getLcd());
                            if (dv != nullptr)
                            {
                                Serial.printf(" ->view created. :%s", dv->getName());
                                vc->setView(dv->getName(), dv);
                                vc->changeNext();
                            }
                        }
                    }
                    Serial.println();
                }
            }
            break;
        default:
            break;
        }
    }
    else
    {
        uint32_t key = deviceId(seoj[0], seoj[1], seoj[2]);
        if (_devices.count(key) != 0)
        {
            _devices[key]->parse(props);
        }
    }
}

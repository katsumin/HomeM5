#include "Device.h"
#include "Node.h"

Device *Device::createInstance(byte eoj0, byte eoj1, byte eoj2, Node *node)
{
    if (eoj0 == 0x00 && eoj1 == 0x11)
        return new TempSensor(eoj0, eoj1, eoj2, node);
    else if (eoj0 == 0x00 && eoj1 == 0x12)
        return new HumiditySensor(eoj0, eoj1, eoj2, node);
    else if (eoj0 == 0x00 && eoj1 == 0x2d)
        return new PressureSensor(eoj0, eoj1, eoj2, node);
    else if (eoj0 == 0x00 && eoj1 == 0x1b)
        return new CO2Sensor(eoj0, eoj1, eoj2, node);
    else if (eoj0 == 0x00 && eoj1 == 0x1d)
        return new VOCSensor(eoj0, eoj1, eoj2, node);
    else if (eoj0 == 0x01 && eoj1 == 0x30)
        return new Aircon(eoj0, eoj1, eoj2, node);
    else if (eoj0 == 0x02 && eoj1 == 0x6b)
        return new ElectricWaterHeater(eoj0, eoj1, eoj2, node);
    else if (eoj0 == 0x02 && eoj1 == 0x88)
        return new SmartMeter(eoj0, eoj1, eoj2, node);
    else if (eoj0 == 0x02 && eoj1 == 0x79)
        return new SolarPower(eoj0, eoj1, eoj2, node);
    else if (eoj0 == 0x02 && eoj1 == 0x7d)
        return new Battery(eoj0, eoj1, eoj2, node);
    else
        return nullptr;
}

// base
Device::Device(byte eoj0, byte eoj1, byte eoj2, Node *node)
{
    setEoj(_eoj, eoj0, eoj1, eoj2);
    _node = node;
}

// Aircon
Aircon::Aircon(byte eoj0, byte eoj1, byte eoj2, Node *node) : Device(eoj0, eoj1, eoj2, node)
{
    setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    _map[0x40] = "その他";
    _map[0x41] = "自動";
    _map[0x42] = "冷房";
    _map[0x43] = "暖房";
    _map[0x44] = "除湿";
    _map[0x45] = "送風";
    _statements[0] = (char *)malloc(BUF_SIZE); // power
    _statements[1] = (char *)malloc(BUF_SIZE); // tempRoom
    _statements[2] = (char *)malloc(BUF_SIZE); // tempOut
    _statements[3] = nullptr;                  // terminate
}

char **Aircon::getInfluxStatement(String key, unsigned long t)
{
    if (_power > 65533)
        return nullptr;
    if (_tempOut > 125 && _tempRoom <= 125)
    {
        snprintf(_statements[1], BUF_SIZE, "power,target=%s,type=aircon value=%d %d000000000", key.c_str(), _power, t);
        snprintf(_statements[2], BUF_SIZE, "temp,target=%s,type=aircon_room value=%d %d000000000", key.c_str(), _tempRoom, t);
        return &_statements[1];
    }
    else if (_tempOut <= 125 && _tempRoom > 125)
    {
        snprintf(_statements[1], BUF_SIZE, "power,target=%s,type=aircon value=%d %d000000000", key.c_str(), _power, t);
        snprintf(_statements[2], BUF_SIZE, "temp,target=%s,type=aircon_out value=%d %d000000000", key.c_str(), _tempOut, t);
        return &_statements[1];
    }
    else if (_tempOut > 125 && _tempRoom > 125)
    {
        snprintf(_statements[2], BUF_SIZE, "power,target=%s,type=aircon value=%d %d000000000", key.c_str(), _power, t);
        return &_statements[2];
    }
    else
    {
        snprintf(_statements[0], BUF_SIZE, "power,target=%s,type=aircon value=%d %d000000000", key.c_str(), _power, t);
        snprintf(_statements[1], BUF_SIZE, "temp,target=%s,type=aircon_room value=%d %d000000000", key.c_str(), _tempRoom, t);
        snprintf(_statements[2], BUF_SIZE, "temp,target=%s,type=aircon_out value=%d %d000000000", key.c_str(), _tempOut, t);
        return _statements;
    }
}

void Aircon::parse(const byte *props)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    switch (ed->epc)
    {
    case 0x80:
        _on = ed->edt[0];
#ifdef DEBUG
        Serial.printf("動作状態: %s", getOn());
        Serial.println();
#endif
        break;
    case 0xb0:
        if (_map.count(ed->edt[0]) > 0)
        {
            _state = ed->edt[0];
#ifdef DEBUG
            Serial.printf("動作モード: %s", getMode());
            Serial.println();
#endif
        }
        break;
    case 0x84:
        if (ed->pdc == 2)
        {
            uint16_t d = edtInt16_t(ed->edt);
            if (d <= 65533)
            {
                _power = d;
#ifdef DEBUG
                Serial.printf("消費電力: %d[w]", _power);
                Serial.println();
#endif
            }
        }
        break;
    case 0xbe:
        if (ed->pdc == 1)
        {
            int8_t d = (int8_t)ed->edt[0];
            if (-127 <= d && d <= 125)
            {
                _tempOut = d;
#ifdef DEBUG
                Serial.printf("外温: %3d[℃]", _tempOut);
                Serial.println();
#endif
            }
        }
        break;
    case 0xbb:
        if (ed->pdc == 1)
        {
            int8_t d = (int8_t)ed->edt[0];
            if (-127 <= d && d <= 125)
            {
                _tempRoom = d;
#ifdef DEBUG
                Serial.printf("室温: %3d[℃]", _tempRoom);
                Serial.println();
#endif
            }
        }
        break;
    default:
        break;
    }
}

// ElectricWaterHeater
ElectricWaterHeater::ElectricWaterHeater(byte eoj0, byte eoj1, byte eoj2, Node *node) : Device(eoj0, eoj1, eoj2, node)
{
    setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    _map[0x41] = "湯張り中";
    _map[0x42] = "停止中";
    _map[0x43] = "保温中";
    _statements[0] = (char *)malloc(BUF_SIZE); // power
    _statements[1] = (char *)malloc(BUF_SIZE); // Tank
    _statements[2] = nullptr;                  // terminate
}

char **ElectricWaterHeater::getInfluxStatement(String key, unsigned long t)
{
    if (_power > 65533)
        return nullptr;
    snprintf(_statements[0], BUF_SIZE, "power,target=%s,type=waterheater value=%d %d000000000", key.c_str(), _power, t);
    snprintf(_statements[1], BUF_SIZE, "tank,target=%s,type=waterheater value=%d %d000000000", key.c_str(), _tank, t);
    return _statements;
}

void ElectricWaterHeater::parse(const byte *props)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    switch (ed->epc)
    {
    case 0xb2:
        _boil = ed->edt[0];
#ifdef DEBUG
        Serial.printf("湧き上げ状態: %s", getBoil());
        Serial.println();
#endif
        break;
    case 0xea:
        if (_map.count(ed->edt[0]) > 0)
        {
            _state = ed->edt[0];
#ifdef DEBUG
            Serial.printf("動作状態: %s", getState());
            Serial.println();
#endif
        }

        break;
    case 0x84:
        if (ed->pdc == 2)
        {
            uint16_t d = edtUInt16_t(ed->edt);
            if (d <= 65533)
            {
                _power = d;
#ifdef DEBUG
                Serial.printf("消費電力: %d[w]", _power);
                Serial.println();
#endif
            }
        }
        break;
    case 0xe1:
        if (ed->pdc == 2)
        {
            uint16_t d = edtUInt16_t(ed->edt);
            if (d <= 65533)
            {
                _tank = d;
#ifdef DEBUG
                Serial.printf("残湯量: %5d[L]", _tank);
                Serial.println();
#endif
            }
        }
        break;
    default:
        break;
    }
}

// SmartMeter
SmartMeter::SmartMeter(byte eoj0, byte eoj1, byte eoj2, Node *node) : Device(eoj0, eoj1, eoj2, node)
{
    setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    _statements[0] = (char *)malloc(BUF_SIZE); // power
    _statements[1] = (char *)malloc(BUF_SIZE); // _wattHourPlus
    _statements[2] = (char *)malloc(BUF_SIZE); // _wattHourMinus
    _statements[3] = nullptr;                  // terminate
}

char **SmartMeter::getInfluxStatement(String key, unsigned long t)
{
    if (_power > 2147483645)
        return nullptr;
    if (_updated)
    {
        _updated = false;
        snprintf(_statements[0], BUF_SIZE, "power,target=%s,type=smartmeter value=%d %d000000000", key.c_str(), _power, t);
        snprintf(_statements[1], BUF_SIZE, "watt_hour,target=%s,type=smartmeter_plus value=%.1f %d000000000", key.c_str(), _wattHourPlus, _timePlus - 9 * 3600);
        snprintf(_statements[2], BUF_SIZE, "watt_hour,target=%s,type=smartmeter_minus value=%.1f %d000000000", key.c_str(), _wattHourMinus, _timeMinus - 9 * 3600);
        return _statements;
    }
    else
    {
        snprintf(_statements[2], BUF_SIZE, "power,target=%s,type=smartmeter value=%d %d000000000", key.c_str(), _power, t);
        return &_statements[2];
    }
}

void SmartMeter::parseEAEB(u_char *edt, time_t *t, float *p)
{
    uint32_t v = edtUInt32_t(&edt[7]);
    if (v <= 99999999L)
    {
        tm tm;
        tm.tm_year = (edt[0] << 8 | edt[1]) - 1900;
        tm.tm_mon = edt[2] - 1;
        tm.tm_mday = edt[3];
        tm.tm_hour = edt[4];
        tm.tm_min = edt[5];
        tm.tm_sec = edt[6];
        *t = mktime(&tm);
        *p = (float)v * _k;
#ifdef DEBUG
        Serial.printf("%04d/%02d/%02d %02d:%02d:%02d, %10.2f[kWh]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, *p);
        Serial.println();
#endif
        _updated = true;
    }
}

void SmartMeter::parse(const byte *props)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    switch (ed->epc)
    {
    case 0xe1:
        switch (ed->edt[0])
        {
        case 0x00:
            _k = 1.0;
            break;
        case 0x01:
            _k = 0.1;
            break;
        case 0x02:
            _k = 0.01;
            break;
        case 0x03:
            _k = 0.001;
            break;
        case 0x04:
            _k = 0.0001;
            break;
        case 0x0a:
            _k = 10.0;
            break;
        case 0x0b:
            _k = 100.0;
            break;
        case 0x0c:
            _k = 1000.0;
            break;
        case 0x0d:
            _k = 10000.0;
            break;
        default:
            break;
        }
    case 0xe7:
        if (ed->pdc == 4)
        {
            int32_t d = edtInt32_t(ed->edt);
            if (-2147483647 <= d && d <= 2147483645)
            {
                _power = d;
#ifdef DEBUG
                Serial.printf("消費電力: %10ld[w]", _power);
                Serial.println();
#endif
            }
        }
        break;
    case 0xea:
        if (ed->pdc == 11)
        {
            parseEAEB(ed->edt, &_timePlus, &_wattHourPlus);
        }
        break;
    case 0xeb:
        if (ed->pdc == 11)
        {
            parseEAEB(ed->edt, &_timeMinus, &_wattHourMinus);
        }
        break;
    default:
        break;
    }
}

// SolarPower
SolarPower::SolarPower(byte eoj0, byte eoj1, byte eoj2, Node *node) : Device(eoj0, eoj1, eoj2, node)
{
    setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    _statements[0] = (char *)malloc(BUF_SIZE); // power
    _statements[1] = (char *)malloc(BUF_SIZE); // _wattHour
    _statements[2] = nullptr;                  // terminate
}

char **SolarPower::getInfluxStatement(String key, unsigned long t)
{
    if (_power > 65533)
        return nullptr;
    snprintf(_statements[0], BUF_SIZE, "power,target=%s,type=solar value=%d %d000000000", key.c_str(), _power, t);
    snprintf(_statements[1], BUF_SIZE, "watt_hour,target=%s,type=solar value=%.3f %d000000000", key.c_str(), _wattHour, (t / (30 * 60) + 1) * ((30 * 60)));
    return _statements;
}

void SolarPower::parse(const byte *props)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    switch (ed->epc)
    {
    case 0xe0:
        if (ed->pdc == 2)
        {
            uint16_t d = edtUInt16_t(ed->edt);
            if (d <= 65533)
            {
                _power = d;
#ifdef DEBUG
                Serial.printf("発電電力: %d[w]", _power);
                Serial.println();
#endif
            }
        }
        break;
    case 0xe1:
        if (ed->pdc == 4)
        {
            uint32_t d = edtUInt32_t(ed->edt);
            if (d <= 999999999L)
            {
                _wattHour = (float)d / 1000.0f;
#ifdef DEBUG
                Serial.printf("積算発電電力量: %11.3f[kWh]", _wattHour);
                Serial.println();
#endif
            }
        }
        break;
    default:
        break;
    }
}

// Battery
Battery::Battery(byte eoj0, byte eoj1, byte eoj2, Node *node) : Device(eoj0, eoj1, eoj2, node)
{
    setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    _map[0x40] = "その他";
    _map[0x41] = "急速充電";
    _map[0x42] = "充電";
    _map[0x43] = "放電";
    _map[0x44] = "待機";
    _map[0x45] = "テスト";
    _map[0x46] = "自動";
    _map[0x48] = "再起動";
    _map[0x49] = "実効容量再計算処理";
    _statements[0] = (char *)malloc(BUF_SIZE); // _percent
    _statements[1] = nullptr;                  // terminate
}

char **Battery::getInfluxStatement(String key, unsigned long t)
{
    if (_percent > 100)
        return nullptr;
    snprintf(_statements[0], BUF_SIZE, "battery,target=%s value=%d %d000000000", key.c_str(), _percent, t);
    return _statements;
}

void Battery::parse(const byte *props)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    switch (ed->epc)
    {
    case 0xcf:
        if (_map.count(ed->edt[0]) > 0)
        {
            _state = ed->edt[0];
#ifdef DEBUG
            Serial.printf("動作状態: %s", getState());
            Serial.println();
#endif
        }
        break;
    case 0xe4:
        if (ed->edt[0] <= 100)
        {
            _percent = ed->edt[0];
#ifdef DEBUG
            Serial.printf("蓄電残量: %3d[%%]", getPercent());
            Serial.println();
#endif
        }
        break;
    default:
        break;
    }
}

// TempSensor
TempSensor::TempSensor(byte eoj0, byte eoj1, byte eoj2, Node *node) : Device(eoj0, eoj1, eoj2, node)
{
    setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    _statements[0] = (char *)malloc(BUF_SIZE); // _value
    _statements[1] = nullptr;                  // terminate
}

char **TempSensor::getInfluxStatement(String key, unsigned long t)
{
    if (_value > 32766)
        return nullptr;
    snprintf(_statements[0], BUF_SIZE, "temp,target=%s,type=sensor value=%.1f %d000000000", key.c_str(), _value, t);
    return _statements;
}

void TempSensor::parse(const byte *props)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    switch (ed->epc)
    {
    case 0xe0:
        if (ed->pdc == 2)
        {
            int16_t d = edtInt16_t(ed->edt);
            if (-2732 <= d && d <= 32766)
            {
                _value = (float)d / 10.0f;
#ifdef DEBUG
                Serial.printf("温度: %6.1f[℃]", _value);
                Serial.println();
#endif
            }
        }
    default:
        break;
    }
}

// HumiditySensor
HumiditySensor::HumiditySensor(byte eoj0, byte eoj1, byte eoj2, Node *node) : Device(eoj0, eoj1, eoj2, node)
{
    setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    _statements[0] = (char *)malloc(BUF_SIZE); // _value
    _statements[1] = nullptr;                  // terminate
}

char **HumiditySensor::getInfluxStatement(String key, unsigned long t)
{
    if (_value == 0xff)
        return nullptr;
    snprintf(_statements[0], BUF_SIZE, "humidity,target=%s,type=sensor value=%d %d000000000", key.c_str(), _value, t);
    return _statements;
}

void HumiditySensor::parse(const byte *props)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    switch (ed->epc)
    {
    case 0xe0:
        if (ed->pdc == 1)
        {
            if (ed->edt[0] <= 100)
            {
                _value = ed->edt[0];
#ifdef DEBUG
                Serial.printf("湿度: %3d[%%]", _value);
                Serial.println();
#endif
            }
        }
    default:
        break;
    }
}

// PressureSensor
PressureSensor::PressureSensor(byte eoj0, byte eoj1, byte eoj2, Node *node) : Device(eoj0, eoj1, eoj2, node)
{
    setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    _statements[0] = (char *)malloc(BUF_SIZE); // _value
    _statements[1] = nullptr;                  // terminate
}

char **PressureSensor::getInfluxStatement(String key, unsigned long t)
{
    if (_value < 0.1)
        return nullptr;
    snprintf(_statements[0], BUF_SIZE, "pressure,target=%s,type=sensor value=%.1f %d000000000", key.c_str(), _value, t);
    return _statements;
}

void PressureSensor::parse(const byte *props)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    switch (ed->epc)
    {
    case 0xe0:
        if (ed->pdc == 2)
        {
            uint16_t d = edtUInt16_t(ed->edt);
            if (d <= 65533)
            {
                _value = (float)d / 10.0f;
#ifdef DEBUG
                Serial.printf("気圧: %6.1f[hPa]", _value);
                Serial.println();
#endif
            }
        }
    default:
        break;
    }
}

// CO2Sensor
CO2Sensor::CO2Sensor(byte eoj0, byte eoj1, byte eoj2, Node *node) : Device(eoj0, eoj1, eoj2, node)
{
    setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    _statements[0] = (char *)malloc(BUF_SIZE); // _value
    _statements[1] = nullptr;                  // terminate
}
char **CO2Sensor::getInfluxStatement(String key, unsigned long t)
{
    if (_value == 0xffff)
        return nullptr;
    snprintf(_statements[0], BUF_SIZE, "co2,target=%s,type=sensor value=%d %d000000000", key.c_str(), _value, t);
    return _statements;
}
void CO2Sensor::parse(const byte *props)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    switch (ed->epc)
    {
    case 0xe0:
        if (ed->pdc == 2)
        {
            uint16_t d = edtUInt16_t(ed->edt);
            if (d <= 65533)
            {
                _value = d;
#ifdef DEBUG
                Serial.printf("CO2: %5d[ppm]", _value);
                Serial.println();
#endif
            }
        }
        break;
    default:
        break;
    }
}

// VOCSensor
VOCSensor::VOCSensor(byte eoj0, byte eoj1, byte eoj2, Node *node) : Device(eoj0, eoj1, eoj2, node)
{
    setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
}

void VOCSensor::parse(const byte *props)
{
    ECHONET_DATA *ed = (ECHONET_DATA *)props;
    switch (ed->epc)
    {
    case 0xe0:
        if (ed->pdc == 2)
        {
            uint16_t d = edtUInt16_t(ed->edt);
            if (d <= 65533)
            {
                _value = d;
                _lValue = _value * 1000 + _subValue;
#ifdef DEBUG
                Serial.printf("VOC: %5d[ppm]", _value);
                Serial.println();
#endif
            }
        }
        break;
    case 0xe1:
        if (ed->pdc == 2)
        {
            uint16_t d = edtUInt16_t(ed->edt);
            if (d <= 999)
            {
                _subValue = d;
                _lValue = _value * 1000 + _subValue;
#ifdef DEBUG
                Serial.printf("VOC: %8d[ppb]", _lValue);
                Serial.println();
#endif
            }
        }
        break;
    default:
        break;
    }
}

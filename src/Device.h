#ifndef _DEVICE_H_
#define _DEVICE_H_
#include <Arduino.h>
#include <map>

#define BUF_SIZE 256
typedef struct
{
    u_char epc;    // 1 byte
    u_char pdc;    // 1 byte
    u_char edt[1]; // n byte (n > 1)
} ECHONET_DATA;

class Node;

class Device
{
private:
    byte _eoj[3];
    Node *_node;

protected:
    int16_t edtInt16_t(byte *edt) { return (int16_t)(edt[0] << 8 | edt[1]); }
    uint16_t edtUInt16_t(byte *edt) { return (uint16_t)(edt[0] << 8 | edt[1]); }
    int32_t edtInt32_t(byte *edt) { return (int32_t)(edt[0] << 24 | edt[1] << 16 | edt[2] << 8 | edt[3]); }
    uint32_t edtUInt32_t(byte *edt) { return (uint32_t)(edt[0] << 24 | edt[1] << 16 | edt[2] << 8 | edt[3]); }

public:
    Device(byte eoj0, byte eoj1, byte eoj2, Node *node);
    /**
     * byte[0]: length
     * byte[1]-byte[length]: payload 
     */
    virtual byte *request();
    virtual void parse(const byte *props);
    inline uint16_t getClassType() { return _eoj[0] << 8 | _eoj[1]; }
    inline Node *getNode() { return _node; }
    inline void setEoj(byte *buf, byte eoj0, byte eoj1, byte eoj2)
    {
        buf[0] = eoj0;
        buf[1] = eoj1;
        buf[2] = eoj2;
    }
    virtual char **getInfluxStatement(String key, unsigned long t) { return nullptr; }
    static Device *createInstance(byte eoj0, byte eoj1, byte eoj2, Node *node);
};

// 家庭用エアコン（0x0130）
class Aircon : public Device
{
private:
    byte _cmd_buf[1 + 2 + 2 + 3 + 3 + 1 + 1 + 5 * 2] = {
        sizeof(_cmd_buf) - 1, //
        0x10, 0x81,           // EHD
        0x00, 0x01,           // TID
        0x05, 0xff, 0x01,     // SEOJ
        0x01, 0x30, 0x01,     // DEOJ
        0x62,                 // ESV
        0x05,                 // OPC
        0x80, 0x00,           // EPC, PDC
        0x84, 0x00,           // EPC, PDC
        0xb0, 0x00,           // EPC, PDC
        0xbe, 0x00,           // EPC, PDC
        0xbb, 0x00,           // EPC, PDC
    };
    uint16_t _power = 0xffff;
    int8_t _tempRoom = 0x7e;
    int8_t _tempOut = 0x7e;
    uint8_t _on = 0x31;
    uint8_t _state = 0x40;
    std::map<uint8_t, char *> _map;
    char *_statements[4];

public:
    Aircon(byte eoj0, byte eoj1, byte eoj2, Node *node);
    inline char *getOn() { return _on == 0x30 ? (char *)"ON" : (char *)"OFF"; }
    inline uint16_t getPower() { return _power; }
    inline int8_t getTempRoom() { return _tempRoom; }
    inline int8_t getTempOut() { return _tempOut; }
    inline char *getMode() { return _map[_state]; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
};

// 電気温水器（0x026b）
class ElectricWaterHeater : public Device
{
private:
    byte _cmd_buf[1 + 2 + 2 + 3 + 3 + 1 + 1 + 4 * 2] = {
        sizeof(_cmd_buf) - 1, //
        0x10, 0x81,           // EHD
        0x00, 0x01,           // TID
        0x05, 0xff, 0x01,     // SEOJ
        0x02, 0x6b, 0x01,     // DEOJ
        0x62,                 // ESV
        0x04,                 // OPC
        0x84, 0x00,           // EPC, PDC
        0xb2, 0x00,           // EPC, PDC
        0xe1, 0x00,           // EPC, PDC
        0xea, 0x00,           // EPC, PDC
    };
    uint16_t _power = 0xffff;
    uint16_t _tank = 0xffff;
    uint8_t _boil = 0x42;
    uint8_t _state = 0x42;
    std::map<uint8_t, char *> _map;
    char *_statements[3];

public:
    ElectricWaterHeater(byte eoj0, byte eoj1, byte eoj2, Node *node);
    inline char *getBoil() { return _boil == 0x41 ? (char *)"ON" : (char *)"OFF"; }
    inline uint16_t getPower() { return _power; }
    inline uint16_t getTank() { return _tank; }
    inline char *getState() { return _map[_state]; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
};

// 低圧スマート電力量メータ（0x0288）
#define WATT_HOUR_POINTS (48)
#define WATT_HOUR_LAST_POINT (WATT_HOUR_POINTS - 1)
class WattHour
{
private:
    time_t _time = 0;
    float _value = 0.0;
    float _values[WATT_HOUR_POINTS]; // 48コマ分

public:
    static int time2Index(time_t epoch);
    inline static int nextIndex(int index) { return (index + 1) % WATT_HOUR_POINTS; };
    inline static int prevIndex(int index) { return (index + WATT_HOUR_LAST_POINT) % WATT_HOUR_POINTS; };
    inline void setTime(time_t t) { _time = t; };
    inline time_t getTime() { return _time; };
    inline void setValue(float v) { _value = v; };
    inline float getValue() { return _value; };
    void init();
    void updateValues(float v, time_t t);
    inline float getValueAtIndex(int index) { return _values[index]; };
};
class SmartMeter : public Device
{
private:
    byte _cmd_buf[1 + 2 + 2 + 3 + 3 + 1 + 1 + 4 * 2] = {
        sizeof(_cmd_buf) - 1, //
        0x10, 0x81,           // EHD
        0x00, 0x01,           // TID
        0x05, 0xff, 0x01,     // SEOJ
        0x02, 0x88, 0x01,     // DEOJ
        0x62,                 // ESV
        0x04,                 // OPC
        0xe7, 0x00,           // EPC, PDC
        0xe1, 0x00,           // EPC, PDC
        0xea, 0x00,           // EPC, PDC
        0xeb, 0x00,           // EPC, PDC
    };
    char _buf[256];
    float _k = 0.1;
    int32_t _power = 0x7fffffff;
    float _wattHourPlus = -1.0;
    time_t _timePlus = 0;
    float _wattHourMinus = -1.0;
    time_t _timeMinus = 0;
    char *_statements[4];
    boolean _updated = false;
    WattHour _plus;
    WattHour _minus;

private:
    void parseEAEB(u_char *edt, time_t *t, float *p);

public:
    SmartMeter(byte eoj0, byte eoj1, byte eoj2, Node *node);
    inline int32_t getPower() { return _power; }
    inline float getWattHourPlus() { return _wattHourPlus; }
    inline float getWattHourMinus() { return _wattHourMinus; }
    inline time_t getTimePlus() { return _timePlus; }
    inline time_t getTimeMinus() { return _timeMinus; }
    inline WattHour getWattHourObjPlus() { return _plus; }
    inline WattHour getWattHourObjMinus() { return _minus; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
};

// 住宅用太陽光発電（0x0279）
class SolarPower : public Device
{
private:
    byte _cmd_buf[1 + 2 + 2 + 3 + 3 + 1 + 1 + 2 * 2] = {
        sizeof(_cmd_buf) - 1, //
        0x10, 0x81,           // EHD
        0x00, 0x01,           // TID
        0x05, 0xff, 0x01,     // SEOJ
        0x02, 0x79, 0x01,     // DEOJ
        0x62,                 // ESV
        0x02,                 // OPC
        0xe0, 0x00,           // EPC, PDC
        0xe1, 0x00,           // EPC, PDC
    };
    uint16_t _power = 0xffff;
    float _wattHour = 0;
    char *_statements[3];

public:
    SolarPower(byte eoj0, byte eoj1, byte eoj2, Node *node);
    inline uint16_t getPower() { return _power; }
    inline float getWattHour() { return _wattHour; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
};

// 蓄電池（0x027d）
class Battery : public Device
{
private:
    byte _cmd_buf[1 + 2 + 2 + 3 + 3 + 1 + 1 + 2 * 2] = {
        sizeof(_cmd_buf) - 1, //
        0x10, 0x81,           // EHD
        0x00, 0x01,           // TID
        0x05, 0xff, 0x01,     // SEOJ
        0x02, 0x7d, 0x01,     // DEOJ
        0x62,                 // ESV
        0x02,                 // OPC
        0xcf, 0x00,           // EPC, PDC（運転動作状態）
        0xe4, 0x00,           // EPC, PDC（蓄電残量３）
    };
    uint8_t _state = 0x40;
    uint8_t _percent = 0xff;
    std::map<uint8_t, char *> _map;
    char *_statements[2];

public:
    Battery(byte eoj0, byte eoj1, byte eoj2, Node *node);
    inline String getState() { return _map[_state]; }
    inline uint8_t getPercent() { return _percent; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
};

// 温度センサ（0x0011）
class TempSensor : public Device
{
private:
    byte _cmd_buf[1 + 2 + 2 + 3 + 3 + 1 + 1 + 1 * 2] = {
        sizeof(_cmd_buf) - 1, //
        0x10, 0x81,           // EHD
        0x00, 0x01,           // TID
        0x05, 0xff, 0x01,     // SEOJ
        0x00, 0x11, 0x01,     // DEOJ
        0x62,                 // ESV
        0x01,                 // OPC
        0xe0, 0x00,           // EPC, PDC
    };
    float _value = 32768.0;
    char *_statements[2];

public:
    TempSensor(byte eoj0, byte eoj1, byte eoj2, Node *node);
    inline float getValue() { return _value; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
};

// 湿度センサ（0x0012）
class HumiditySensor : public Device
{
private:
    byte _cmd_buf[1 + 2 + 2 + 3 + 3 + 1 + 1 + 1 * 2] = {
        sizeof(_cmd_buf) - 1, //
        0x10, 0x81,           // EHD
        0x00, 0x01,           // TID
        0x05, 0xff, 0x01,     // SEOJ
        0x00, 0x12, 0x01,     // DEOJ
        0x62,                 // ESV
        0x01,                 // OPC
        0xe0, 0x00,           // EPC, PDC
    };
    uint8_t _value = 0xff;
    char *_statements[2];

public:
    HumiditySensor(byte eoj0, byte eoj1, byte eoj2, Node *node);
    inline uint8_t getValue() { return _value; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
};

// 気圧センサ（0x002d）
class PressureSensor : public Device
{
private:
    byte _cmd_buf[1 + 2 + 2 + 3 + 3 + 1 + 1 + 1 * 2] = {
        sizeof(_cmd_buf) - 1, //
        0x10, 0x81,           // EHD
        0x00, 0x01,           // TID
        0x05, 0xff, 0x01,     // SEOJ
        0x00, 0x2d, 0x01,     // DEOJ
        0x62,                 // ESV
        0x01,                 // OPC
        0xe0, 0x00,           // EPC, PDC
    };
    float _value = 0;
    char *_statements[2];

public:
    PressureSensor(byte eoj0, byte eoj1, byte eoj2, Node *node);
    inline float getValue() { return _value; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
};

// CO2センサ（0x001b）
class CO2Sensor : public Device
{
private:
    byte _cmd_buf[1 + 2 + 2 + 3 + 3 + 1 + 1 + 1 * 2] = {
        sizeof(_cmd_buf) - 1, //
        0x10, 0x81,           // EHD
        0x00, 0x01,           // TID
        0x05, 0xff, 0x01,     // SEOJ
        0x00, 0x1b, 0x01,     // DEOJ
        0x62,                 // ESV
        0x01,                 // OPC
        0xe0, 0x00,           // EPC, PDC
    };
    uint16_t _value = 0xffff;
    char *_statements[2];

public:
    CO2Sensor(byte eoj0, byte eoj1, byte eoj2, Node *node);
    inline uint16_t getValue() { return _value; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
};

// VOCセンサ（0x001d）
class VOCSensor : public Device
{
private:
    byte _cmd_buf[1 + 2 + 2 + 3 + 3 + 1 + 1 + 2 * 2] = {
        sizeof(_cmd_buf) - 1, //
        0x10, 0x81,           // EHD
        0x00, 0x01,           // TID
        0x05, 0xff, 0x01,     // SEOJ
        0x00, 0x1d, 0x01,     // DEOJ
        0x62,                 // ESV
        0x02,                 // OPC
        0xe0, 0x00,           // EPC, PDC
        0xe1, 0x00,           // EPC, PDC
    };
    uint16_t _value = 0xffff;
    uint16_t _subValue = 0;
    uint32_t _lValue = 0xffffffff;

public:
    VOCSensor(byte eoj0, byte eoj1, byte eoj2, Node *node);
    inline uint16_t getValue() { return _value; }
    inline uint32_t getLValue() { return _lValue; }
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
};

#endif
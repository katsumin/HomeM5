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
    // {
    //     if (_power > 65533)
    //         return nullptr;
    //     if (_tempOut > 125 && _tempRoom <= 125)
    //     {
    //         snprintf(_statements[1], BUF_SIZE, "power,target=%s,type=aircon value=%d %d000000000", key.c_str(), _power, t);
    //         snprintf(_statements[2], BUF_SIZE, "temp,target=%s,type=aircon_room value=%d %d000000000", key.c_str(), _tempRoom, t);
    //         return &_statements[1];
    //     }
    //     else if (_tempOut <= 125 && _tempRoom > 125)
    //     {
    //         snprintf(_statements[1], BUF_SIZE, "power,target=%s,type=aircon value=%d %d000000000", key.c_str(), _power, t);
    //         snprintf(_statements[2], BUF_SIZE, "temp,target=%s,type=aircon_out value=%d %d000000000", key.c_str(), _tempOut, t);
    //         return &_statements[1];
    //     }
    //     else if (_tempOut > 125 && _tempRoom > 125)
    //     {
    //         snprintf(_statements[2], BUF_SIZE, "power,target=%s,type=aircon value=%d %d000000000", key.c_str(), _power, t);
    //         return &_statements[2];
    //     }
    //     else
    //     {
    //         snprintf(_statements[0], BUF_SIZE, "power,target=%s,type=aircon value=%d %d000000000", key.c_str(), _power, t);
    //         snprintf(_statements[1], BUF_SIZE, "temp,target=%s,type=aircon_room value=%d %d000000000", key.c_str(), _tempRoom, t);
    //         snprintf(_statements[2], BUF_SIZE, "temp,target=%s,type=aircon_out value=%d %d000000000", key.c_str(), _tempOut, t);
    //         return _statements;
    //     }
    // }
    virtual byte *request()
    {
        return _cmd_buf;
    }
    virtual void parse(const byte *props);
    //     {
    //         ECHONET_DATA *ed = (ECHONET_DATA *)props;
    //         switch (ed->epc)
    //         {
    //         case 0x80:
    //             _on = ed->edt[0];
    // #ifdef DEBUG
    //             Serial.printf("動作状態: %s", getOn());
    //             Serial.println();
    // #endif
    //             break;
    //         case 0xb0:
    //             if (_map.count(ed->edt[0]) > 0)
    //             {
    //                 _state = ed->edt[0];
    // #ifdef DEBUG
    //                 Serial.printf("動作モード: %s", getMode());
    //                 Serial.println();
    // #endif
    //             }
    //             break;
    //         case 0x84:
    //             if (ed->pdc == 2)
    //             {
    //                 uint16_t d = edtInt16_t(ed->edt);
    //                 if (d <= 65533)
    //                 {
    //                     _power = d;
    // #ifdef DEBUG
    //                     Serial.printf("消費電力: %d[w]", _power);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         case 0xbe:
    //             if (ed->pdc == 1)
    //             {
    //                 int8_t d = (int8_t)ed->edt[0];
    //                 if (-127 <= d && d <= 125)
    //                 {
    //                     _tempOut = d;
    // #ifdef DEBUG
    //                     Serial.printf("外温: %3d[℃]", _tempOut);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         case 0xbb:
    //             if (ed->pdc == 1)
    //             {
    //                 int8_t d = (int8_t)ed->edt[0];
    //                 if (-127 <= d && d <= 125)
    //                 {
    //                     _tempRoom = d;
    // #ifdef DEBUG
    //                     Serial.printf("室温: %3d[℃]", _tempRoom);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         default:
    //             break;
    //         }
    //     }
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
    // {
    //     setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    //     _map[0x41] = "湯張り中";
    //     _map[0x42] = "停止中";
    //     _map[0x43] = "保温中";
    //     _statements[0] = (char *)malloc(BUF_SIZE); // power
    //     _statements[1] = (char *)malloc(BUF_SIZE); // Tank
    //     _statements[2] = nullptr;                  // terminate
    // }
    inline char *getBoil() { return _boil == 0x41 ? (char *)"ON" : (char *)"OFF"; }
    inline uint16_t getPower() { return _power; }
    inline uint16_t getTank() { return _tank; }
    inline char *getState() { return _map[_state]; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    // {
    //     if (_power > 65533)
    //         return nullptr;
    //     snprintf(_statements[0], BUF_SIZE, "power,target=%s,type=waterheater value=%d %d000000000", key.c_str(), _power, t);
    //     snprintf(_statements[1], BUF_SIZE, "tank,target=%s,type=waterheater value=%d %d000000000", key.c_str(), _tank, t);
    //     return _statements;
    // }
    virtual byte *request()
    {
        return _cmd_buf;
    }
    virtual void parse(const byte *props);
    //     {
    //         ECHONET_DATA *ed = (ECHONET_DATA *)props;
    //         switch (ed->epc)
    //         {
    //         case 0xb2:
    //             _boil = ed->edt[0];
    // #ifdef DEBUG
    //             Serial.printf("湧き上げ状態: %s", getBoil());
    //             Serial.println();
    // #endif
    //             break;
    //         case 0xea:
    //             if (_map.count(ed->edt[0]) > 0)
    //             {
    //                 _state = ed->edt[0];
    // #ifdef DEBUG
    //                 Serial.printf("動作状態: %s", getState());
    //                 Serial.println();
    // #endif
    //             }

    //             break;
    //         case 0x84:
    //             if (ed->pdc == 2)
    //             {
    //                 uint16_t d = edtUInt16_t(ed->edt);
    //                 if (d <= 65533)
    //                 {
    //                     _power = d;
    // #ifdef DEBUG
    //                     Serial.printf("消費電力: %d[w]", _power);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         case 0xe1:
    //             if (ed->pdc == 2)
    //             {
    //                 uint16_t d = edtUInt16_t(ed->edt);
    //                 if (d <= 65533)
    //                 {
    //                     _tank = d;
    // #ifdef DEBUG
    //                     Serial.printf("残湯量: %5d[L]", _tank);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         default:
    //             break;
    //         }
    //     }
};

// 低圧スマート電力量メータ（0x0288）
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

private:
    void parseEAEB(u_char *edt, time_t *t, float *p);
    //     {
    //         uint32_t v = edtUInt32_t(&edt[7]);
    //         if (v <= 99999999L)
    //         {
    //             tm tm;
    //             tm.tm_year = (edt[0] << 8 | edt[1]) - 1900;
    //             tm.tm_mon = edt[2] - 1;
    //             tm.tm_mday = edt[3];
    //             tm.tm_hour = edt[4];
    //             tm.tm_min = edt[5];
    //             tm.tm_sec = edt[6];
    //             *t = mktime(&tm);
    //             *p = (float)v * _k;
    // #ifdef DEBUG
    //             Serial.printf("%04d/%02d/%02d %02d:%02d:%02d, %10.2f[kWh]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, *p);
    //             Serial.println();
    // #endif
    //             _updated = true;
    //         }
    //     }

public:
    SmartMeter(byte eoj0, byte eoj1, byte eoj2, Node *node);
    // {
    //     setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    //     _statements[0] = (char *)malloc(BUF_SIZE); // power
    //     _statements[1] = (char *)malloc(BUF_SIZE); // _wattHourPlus
    //     _statements[2] = (char *)malloc(BUF_SIZE); // _wattHourMinus
    //     _statements[3] = nullptr;                  // terminate
    // }
    inline int32_t getPower() { return _power; }
    inline float getWattHourPlus() { return _wattHourPlus; }
    inline float getWattHourMinus() { return _wattHourMinus; }
    inline time_t getTimePlus() { return _timePlus; }
    inline time_t getTimeMinus() { return _timeMinus; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    // {
    //     if (_power > 2147483645)
    //         return nullptr;
    //     if (_updated)
    //     {
    //         _updated = false;
    //         snprintf(_statements[0], BUF_SIZE, "power,target=%s,type=smartmeter value=%d %d000000000", key.c_str(), _power, t);
    //         snprintf(_statements[1], BUF_SIZE, "watt_hour,target=%s,type=smartmeter_plus value=%.1f %d000000000", key.c_str(), _wattHourPlus, _timePlus - 9 * 3600);
    //         snprintf(_statements[2], BUF_SIZE, "watt_hour,target=%s,type=smartmeter_minus value=%.1f %d000000000", key.c_str(), _wattHourMinus, _timeMinus - 9 * 3600);
    //         return _statements;
    //     }
    //     else
    //     {
    //         snprintf(_statements[2], BUF_SIZE, "power,target=%s,type=smartmeter value=%d %d000000000", key.c_str(), _power, t);
    //         return &_statements[2];
    //     }
    // }
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
    //     {
    //         ECHONET_DATA *ed = (ECHONET_DATA *)props;
    //         switch (ed->epc)
    //         {
    //         case 0xe1:
    //             switch (ed->edt[0])
    //             {
    //             case 0x00:
    //                 _k = 1.0;
    //                 break;
    //             case 0x01:
    //                 _k = 0.1;
    //                 break;
    //             case 0x02:
    //                 _k = 0.01;
    //                 break;
    //             case 0x03:
    //                 _k = 0.001;
    //                 break;
    //             case 0x04:
    //                 _k = 0.0001;
    //                 break;
    //             case 0x0a:
    //                 _k = 10.0;
    //                 break;
    //             case 0x0b:
    //                 _k = 100.0;
    //                 break;
    //             case 0x0c:
    //                 _k = 1000.0;
    //                 break;
    //             case 0x0d:
    //                 _k = 10000.0;
    //                 break;
    //             default:
    //                 break;
    //             }
    //         case 0xe7:
    //             if (ed->pdc == 4)
    //             {
    //                 int32_t d = edtInt32_t(ed->edt);
    //                 if (-2147483647 <= d && d <= 2147483645)
    //                 {
    //                     _power = d;
    // #ifdef DEBUG
    //                     Serial.printf("消費電力: %10ld[w]", _power);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         case 0xea:
    //             if (ed->pdc == 11)
    //             {
    //                 parseEAEB(ed->edt, &_timePlus, &_wattHourPlus);
    //             }
    //             break;
    //         case 0xeb:
    //             if (ed->pdc == 11)
    //             {
    //                 parseEAEB(ed->edt, &_timeMinus, &_wattHourMinus);
    //             }
    //             break;
    //         default:
    //             break;
    //         }
    //     }
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
    // {
    //     setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    //     _statements[0] = (char *)malloc(BUF_SIZE); // power
    //     _statements[1] = (char *)malloc(BUF_SIZE); // _wattHour
    //     _statements[2] = nullptr;                  // terminate
    // }
    inline uint16_t getPower() { return _power; }
    inline float getWattHour() { return _wattHour; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    // {
    //     if (_power > 65533)
    //         return nullptr;
    //     snprintf(_statements[0], BUF_SIZE, "power,target=%s,type=solar value=%d %d000000000", key.c_str(), _power, t);
    //     snprintf(_statements[1], BUF_SIZE, "watt_hour,target=%s,type=solar value=%.3f %d000000000", key.c_str(), _wattHour, (t / (30 * 60) + 1) * ((30 * 60)));
    //     return _statements;
    // }
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
    //     {
    //         ECHONET_DATA *ed = (ECHONET_DATA *)props;
    //         switch (ed->epc)
    //         {
    //         case 0xe0:
    //             if (ed->pdc == 2)
    //             {
    //                 uint16_t d = edtUInt16_t(ed->edt);
    //                 if (d <= 65533)
    //                 {
    //                     _power = d;
    // #ifdef DEBUG
    //                     Serial.printf("発電電力: %d[w]", _power);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         case 0xe1:
    //             if (ed->pdc == 4)
    //             {
    //                 uint32_t d = edtUInt32_t(ed->edt);
    //                 if (d <= 999999999L)
    //                 {
    //                     _wattHour = (float)d / 1000.0f;
    // #ifdef DEBUG
    //                     Serial.printf("積算発電電力量: %11.3f[kWh]", _wattHour);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         default:
    //             break;
    //         }
    //     }
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
    // {
    //     setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    //     _map[0x40] = "その他";
    //     _map[0x41] = "急速充電";
    //     _map[0x42] = "充電";
    //     _map[0x43] = "放電";
    //     _map[0x44] = "待機";
    //     _map[0x45] = "テスト";
    //     _map[0x46] = "自動";
    //     _map[0x48] = "再起動";
    //     _map[0x49] = "実効容量再計算処理";
    //     _statements[0] = (char *)malloc(BUF_SIZE); // _percent
    //     _statements[1] = nullptr;                  // terminate
    // }
    inline String getState() { return _map[_state]; }
    inline uint8_t getPercent() { return _percent; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    // {
    //     if (_percent > 100)
    //         return nullptr;
    //     snprintf(_statements[0], BUF_SIZE, "battery,target=%s value=%d %d000000000", key.c_str(), _percent, t);
    //     return _statements;
    // }
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
    //     {
    //         ECHONET_DATA *ed = (ECHONET_DATA *)props;
    //         switch (ed->epc)
    //         {
    //         case 0xcf:
    //             if (_map.count(ed->edt[0]) > 0)
    //             {
    //                 _state = ed->edt[0];
    // #ifdef DEBUG
    //                 Serial.printf("動作状態: %s", _map[_state]);
    //                 Serial.println();
    // #endif
    //             }
    //             break;
    //         case 0xe4:
    //             if (ed->edt[0] <= 100)
    //             {
    //                 _percent = ed->edt[0];
    // #ifdef DEBUG
    //                 Serial.printf("蓄電残量: %3d[%%]", _percent);
    //                 Serial.println();
    // #endif
    //             }
    //             break;
    //         default:
    //             break;
    //         }
    //     }
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
    // {
    //     setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    //     _statements[0] = (char *)malloc(BUF_SIZE); // _value
    //     _statements[1] = nullptr;                  // terminate
    // }
    inline float getValue() { return _value; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    // {
    //     if (_value > 32766)
    //         return nullptr;
    //     snprintf(_statements[0], BUF_SIZE, "temp,target=%s,type=sensor value=%.1f %d000000000", key.c_str(), _value, t);
    //     return _statements;
    // }
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
    //     {
    //         ECHONET_DATA *ed = (ECHONET_DATA *)props;
    //         switch (ed->epc)
    //         {
    //         case 0xe0:
    //             if (ed->pdc == 2)
    //             {
    //                 int16_t d = edtInt16_t(ed->edt);
    //                 if (-2732 <= d && d <= 32766)
    //                 {
    //                     _value = (float)d / 10.0f;
    // #ifdef DEBUG
    //                     Serial.printf("温度: %6.1f[℃]", _value);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //         default:
    //             break;
    //         }
    //     }
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
    // {
    //     setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    //     _statements[0] = (char *)malloc(BUF_SIZE); // _value
    //     _statements[1] = nullptr;                  // terminate
    // }
    inline uint8_t getValue() { return _value; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    // {
    //     if (_value == 0xff)
    //         return nullptr;
    //     snprintf(_statements[0], BUF_SIZE, "humidity,target=%s,type=sensor value=%d %d000000000", key.c_str(), _value, t);
    //     return _statements;
    // }
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
    //     {
    //         ECHONET_DATA *ed = (ECHONET_DATA *)props;
    //         switch (ed->epc)
    //         {
    //         case 0xe0:
    //             if (ed->pdc == 1)
    //             {
    //                 if (ed->edt[0] <= 100)
    //                 {
    //                     _value = ed->edt[0];
    // #ifdef DEBUG
    //                     Serial.printf("湿度: %3d[%%]", _value);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //         default:
    //             break;
    //         }
    //     }
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
    // {
    //     setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    //     _statements[0] = (char *)malloc(BUF_SIZE); // _value
    //     _statements[1] = nullptr;                  // terminate
    // }
    inline float getValue() { return _value; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    // {
    //     if (_value < 0.1)
    //         return nullptr;
    //     snprintf(_statements[0], BUF_SIZE, "pressure,target=%s,type=sensor value=%.1f %d000000000", key.c_str(), _value, t);
    //     return _statements;
    // }
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
    //     {
    //         ECHONET_DATA *ed = (ECHONET_DATA *)props;
    //         switch (ed->epc)
    //         {
    //         case 0xe0:
    //             if (ed->pdc == 2)
    //             {
    //                 uint16_t d = edtUInt16_t(ed->edt);
    //                 if (d <= 65533)
    //                 {
    //                     _value = (float)d / 10.0f;
    // #ifdef DEBUG
    //                     Serial.printf("気圧: %6.1f[hPa]", _value);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //         default:
    //             break;
    //         }
    //     }
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
    // {
    //     setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    //     _statements[0] = (char *)malloc(BUF_SIZE); // _value
    //     _statements[1] = nullptr;                  // terminate
    // }
    inline uint16_t getValue() { return _value; }
    virtual char **getInfluxStatement(String key, unsigned long t);
    // {
    //     if (_value == 0xffff)
    //         return nullptr;
    //     snprintf(_statements[0], BUF_SIZE, "co2,target=%s,type=sensor value=%d %d000000000", key.c_str(), _value, t);
    //     return _statements;
    // }
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
    //     {
    //         ECHONET_DATA *ed = (ECHONET_DATA *)props;
    //         switch (ed->epc)
    //         {
    //         case 0xe0:
    //             if (ed->pdc == 2)
    //             {
    //                 uint16_t d = edtUInt16_t(ed->edt);
    //                 if (d <= 65533)
    //                 {
    //                     _value = d;
    // #ifdef DEBUG
    //                     Serial.printf("CO2: %5d[ppm]", _value);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         default:
    //             break;
    //         }
    //     }
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
    // {
    //     setEoj(&_cmd_buf[8], eoj0, eoj1, eoj2);
    // }
    inline uint16_t getValue() { return _value; }
    inline uint32_t getLValue() { return _lValue; }
    virtual byte *request() { return _cmd_buf; }
    virtual void parse(const byte *props);
    //     {
    //         ECHONET_DATA *ed = (ECHONET_DATA *)props;
    //         switch (ed->epc)
    //         {
    //         case 0xe0:
    //             if (ed->pdc == 2)
    //             {
    //                 uint16_t d = edtUInt16_t(ed->edt);
    //                 if (d <= 65533)
    //                 {
    //                     _value = d;
    //                     _lValue = _value * 1000 + _subValue;
    // #ifdef DEBUG
    //                     Serial.printf("VOC: %5d[ppm]", _value);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         case 0xe1:
    //             if (ed->pdc == 2)
    //             {
    //                 uint16_t d = edtUInt16_t(ed->edt);
    //                 if (d <= 999)
    //                 {
    //                     _subValue = d;
    //                     _lValue = _value * 1000 + _subValue;
    // #ifdef DEBUG
    //                     Serial.printf("VOC: %8d[ppb]", _lValue);
    //                     Serial.println();
    // #endif
    //                 }
    //             }
    //             break;
    //         default:
    //             break;
    //         }
    //     }
};

#endif
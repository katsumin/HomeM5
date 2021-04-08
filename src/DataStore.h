#ifndef _DATASTORE_H_
#define _DATASTORE_H_
// #include <M5Stack.h>
#include <map>
#include "Node.h"
#include "EL.h"
#include "InfluxDb.h"
#include "DeviceView.h"

class DataStore
{
private:
    EL *_echo;
    std::map<String, String> _keys;
    std::map<String, Node *> _nodes;
    InfluxDb *_influxdb;
    ViewController *_viewController;
    // // 気温
    // float _temperature = 0;
    // // 湿度
    // float _humidity = 0;
    // // 気圧
    // float _pressure = 0;
    // // 瞬時電力
    // long _power = 0;
    // // 正方向積算電力量
    // WattHour _plus;
    // // 負方向積算電力量
    // WattHour _minus;
    // ecocute-power
    // long _ecoPower = 0;
    // // ecocute-tank
    // long _ecoTank = 0;
    // // air-con power
    // long _airPower = 0;
    // // air-con temp. room
    // float _tempRoom = 0;
    // // air-con temp. out
    // float _tempOut = 0;

public:
    DataStore(InfluxDb *db, ViewController *vc)
    {
        setInfluxdb(db);
        setViewController(vc);
    }
    inline void setEchonet(EL *el) { _echo = el; }
    inline InfluxDb *getInfluxdb() { return _influxdb; }
    inline void setInfluxdb(InfluxDb *db) { _influxdb = db; }
    inline ViewController *getViewController() { return _viewController; }
    inline void setViewController(ViewController *vc) { _viewController = vc; }
    inline std::map<String, Node *> *getNodes() { return &_nodes; }
    void processingProperty(const byte *props, IPAddress addr, const byte *seoj)
    {
        String key = addr.toString();
        // ノードプロファイルのプロパティ
        if (_keys.count(key) > 0)
            key = _keys[key];
        else
        {
            // 暫定キーとしてIPアドレスを使う
            _keys[key] = key;
            _nodes[key] = new Node(addr, this);
            _nodes[key]->setEchonet(_echo);
#ifdef DEBUG
            Serial.printf("node count:%d", _nodes.size());
            Serial.println();
#endif
        }

        // Nodeに受信電文のパースを委譲
        Node *n = _nodes[key];
        n->parse(props, seoj);

        // 識別番号を正式なキーとして更新
        String id = n->getId();
        if (!id.isEmpty() && key != id)
        {
#ifdef DEBUG
            Serial.printf("id: %s", id.c_str());
            Serial.println();
#endif
            _keys[key] = id;
            _nodes[id] = n;
            _nodes.erase(key);
        }
    }
    void request()
    {
        for (auto itr = _nodes.begin(); itr != _nodes.end(); ++itr)
        {
            itr->second->request();
            delay(10);
        }
    }
    void updateInflux(unsigned long t)
    {
        for (auto itr = _nodes.begin(); itr != _nodes.end(); ++itr)
            itr->second->updateInflux(t);
    }

    // inline void setTemperature(float temp)
    // {
    //     _temperature = temp;
    // };
    // inline float getTemperature() { return _temperature; };
    // inline void setHumidity(float hum) { _humidity = hum; };
    // inline float getHumidity() { return _humidity; };
    // inline void setPressure(float press) { _pressure = press; };
    // inline float getPressure() { return _pressure; };
    // inline void setPower(long power) { _power = power; };
    // inline long getPower() { return _power; };
    // void init()
    // {
    // _plus.init();
    // _minus.init();
    // }
    // inline void setWattHourPlus(float plus, time_t t) { _plus.updateValues(plus, t); };
    // inline time_t getWattHourPlusTime() { return _plus.getTime(); };
    // inline float getWattHourPlus() { return _plus.getValue(); };
    // inline float getWattHourPlusAtIndex(int index) { return _plus.getValueAtIndex(index); };
    // inline void setWattHourMinus(float minus, time_t t) { _minus.updateValues(minus, t); };
    // inline time_t getWattHourMinusTime() { return _minus.getTime(); };
    // inline float getWattHourMinus() { return _minus.getValue(); };
    // inline float getWattHourMinusAtIndex(int index) { return _minus.getValueAtIndex(index); };
    // inline void setEcoPower(long power) { _ecoPower = power; };
    // inline long getEcoPower() { return _ecoPower; };
    // inline void setEcoTank(long tank) { _ecoTank = tank; };
    // inline long getEcoTank() { return _ecoTank; };
    // inline void setAirPower(long power) { _airPower = power; };
    // inline long getAirPower() { return _airPower; };
    // inline void setAirTempRoom(float temp) { _tempRoom = temp; };
    // inline float getAirTempRoom() { return _tempRoom; };
    // inline void setAirTempOut(float temp) { _tempOut = temp; };
    // inline float getAirTempOut() { return _tempOut; };
};

#endif
#ifndef _DATASTORE_H_
#define _DATASTORE_H_
#include <map>
#include <PubSubClient.h>
#include "Node.h"
#include "EL.h"
// #include "InfluxDb.h"
#include "DeviceView.h"

class DataStore
{
private:
    EL *_echo;
    std::map<String, String> _keys;
    std::map<String, Node *> _nodes;
    // InfluxDb *_influxdb;
    ViewController *_viewController;
    xSemaphoreHandle _mutex;
    PubSubClient *_mqtt;

public:
    DataStore(ViewController *vc)
    {
        setViewController(vc);
    }
    // void init(Client *pC, const char *broker_address, int port, uint16_t bufer_size, const char *topic)
    // {
    // InfluxDb *db = new InfluxDb(influx_server, influx_db);
    // db->init(pC);
    // setInfluxdb(db);
    // PubSubClient *mqtt = new PubSubClient(broker_address, port, *pC);
    // mqtt->setBufferSize(bufer_size);
    // setMqtt(mqtt);
    // }
    inline void setEchonet(EL *el) { _echo = el; }
    // inline InfluxDb *getInfluxdb() { return _influxdb; }
    // inline void setInfluxdb(InfluxDb *db) { _influxdb = db; }
    inline ViewController *getViewController() { return _viewController; }
    inline void setViewController(ViewController *vc) { _viewController = vc; }
    inline void setMutex(xSemaphoreHandle mutex) { _mutex = mutex; }
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
            delay(100);
        }
    }
    void updateInflux(unsigned long t, const char *topic)
    {
        // std::string st = "";
        for (auto itr = _nodes.begin(); itr != _nodes.end(); ++itr)
        {
            std::string statement = itr->second->updateInflux(t);
            if (statement.length() == 0)
                continue;
            // st.append(statement);
            xSemaphoreTake(_mutex, portMAX_DELAY);
            Serial.printf("mqtt data len: %d\n", statement.length());
            boolean res = getMqtt()->publish(topic, statement.c_str());
            Serial.printf("mqtt publish: %d\n", res);
            xSemaphoreGive(_mutex);
            delay(100); // 0.1s wait
        }
    }
    inline PubSubClient *getMqtt() { return _mqtt; }
    inline void setMqtt(PubSubClient *pC) { _mqtt = pC; }
};

#endif
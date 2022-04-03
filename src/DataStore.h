#ifndef _DATASTORE_H_
#define _DATASTORE_H_
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

public:
    DataStore(ViewController *vc)
    {
        setViewController(vc);
    }
    void init(Client *pC, const char *influx_server, const char *influx_db)
    {
        InfluxDb *db = new InfluxDb(influx_server, influx_db);
        db->init(pC);
        setInfluxdb(db);
    }
    inline void setEchonet(EL *el) { _echo = el; }
    inline InfluxDb *getInfluxdb() { return _influxdb; }
    inline void setInfluxdb(InfluxDb *db) { _influxdb = db; }
    inline ViewController *getViewController() { return _viewController; }
    inline void setViewController(ViewController *vc) { _viewController = vc; }
    inline void setMutex(xSemaphoreHandle mutex) { _influxdb->setMutex(mutex); }
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
    void updateInflux(unsigned long t)
    {
        std::string st = "";
        for (auto itr = _nodes.begin(); itr != _nodes.end(); ++itr)
        {
            std::string statement = itr->second->updateInflux(t);
            if (statement.length() == 0)
                continue;
            st.append(statement);
        }
        getInfluxdb()->write(st.c_str());
    }
};

#endif
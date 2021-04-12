```plantuml
object "Browser" as User
rectangle "Server"{
  object "Grafana" as Grafana
  object "InfluxDB" as InfluxDB
}
rectangle "宅内" {
    object "本装置(M5Stack)" as M5Stack
    rectangle "Echonet Lite機器"{
        object "Wi-SUNゲートウェイ" as WisunGW
        object "エアコン" as Aircon{
            - 瞬時電力
            - 気温（室内、室外）
        }
        object "太陽光発電" as Solar{
            - 瞬時電力
            - 積算電力量
        }
        object "蓄電池" as Battery{
            - 蓄電残量
        }
        object "エコキュート" as Ecocute {
            - 瞬時電力
            - 残湯量
        }
        object "センサ" as Sensor{
            - 気温
            - 湿度
            - 気圧
            - CO2
            - VOC
        }
    }
}
object "スマートメータ" as smartmeter{
    - 瞬時電力
    - 積算電力量
}

User -r-> Grafana : Webアクセス
Grafana <-r- InfluxDB : DB値をグラフ化　
M5Stack -u-> InfluxDB : 定期的にupload
M5Stack -d-> WisunGW
WisunGW -d-> smartmeter
M5Stack -d-> Aircon
M5Stack -d-> Solar : 定期的に情報取得
M5Stack -d-> Battery
M5Stack -d-> Ecocute
M5Stack -d-> Sensor
```

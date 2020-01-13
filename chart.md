```plantuml

object "Browser" as User{

}
rectangle "Server"{
  object "Grafana" as Grafana{
  }
  object "InfluxDB" as InfluxDB{
  }
}
object "本装置(M5Stack)" as M5Stack{

}
object "Wi-SUNゲートウェイ" as WisunGW{

}

User -r-> Grafana : Webアクセス
Grafana <-l- InfluxDB : DB値をグラフ化　
M5Stack -u-> InfluxDB : 定期的にupload
M5Stack <-r-> WisunGW : スマートメータ\n情報の取得
```

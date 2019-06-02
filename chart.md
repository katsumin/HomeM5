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

User -r-> Grafana : Webアクセス
Grafana <-l- InfluxDB : DB値をグラフ化　
M5Stack -u-> InfluxDB : 定期的にupload
```

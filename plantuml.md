@startuml
state "切断状態" as disconnected
state "スキャン中" as scanning
state "接続処理中" as connecting
state "接続状態" as joined
state "切断コマンド送信" as terminate

[*] -r-> disconnected : "電源 ON"
disconnected -r-> scanning : "JOIN ボタン"
scanning --> scanning : "スマートメーター情報取得"
scanning -d-> connecting : "情報取得完了"
connecting --> joined : "接続完了通知イベント受信"
joined -r-> joined : "計測データ／イベント受信"
joined --> terminate : "切断要求イベント受信"
terminate --> scanning : "１秒後"
joined -u-> disconnected : "TERM ボタン"
@enduml

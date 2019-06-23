#ifndef _SMARTMETER_H_
#define _SMARTMETER_H_

#include <Arduino.h>
#include "echonet.h"
#include "debugView.h"
#define BP35C0_CHANNEL 0
#define BP35C0_CHANNEL_PAGE 1
#define BP35C0_PAN_ID 2
#define BP35C0_ADDR 3
#define BP35C0_LQI 4
#define BP35C0_SIDE 5
#define BP35C0_PAIR_ID 6
#define BP35C0_RSTn GPIO_NUM_5
#define BP35C0_bps 115200

enum RCV_CODE
{
  // タイムアウト
  TIMEOUT,
  // UDP受信
  ERXUDP,
  // UDP受信/瞬時電力
  ERXUDP_E7,
  // UDP受信/積算電力量
  ERXUDP_EAB,
  // PANAによる接続が完了
  EVENT_25,
  // 接続相手からセッション終了要求を受信した
  EVENT_26,
  // PANA セッションの終了に成功した
  EVENT_27,
  // PANA セッションの終了要求に対する応答がなくタイムアウトした (セッションは終了)
  EVENT_28,
  // セッション期限切れ
  EVENT_29,
};

enum CONNECT_STATE
{
  // スキャン中
  SCANNING,
  // 接続中
  CONNECTING,
  // 接続済み
  CONNECTED,
  // 切断済み
  DISCONNECTED,
};

class SmartMeter : public Echonet
{
private:
  char _para[7][20];
  char _addr[64];
  static u_char _cmd_buf[];
  float _k = 0.1;
  long _power;
  float _powerPlus;
  float _powerMinus;
  time_t _timePlus;
  time_t _timeMinus;
  DebugView *_debugView = NULL;
  CONNECT_STATE _connectState;

private:
  void parseE1(u_char *edt);
  void parseE7(u_char *edt);
  void parseEAEB(u_char *edt, time_t *t, float *p);
  /* 
  チャンネル・スキャン
  [引数]
   なし
  [戻り値]
   void
  [副作用]
   ・_para配列に通知されたPAN情報が格納される
  */
  void scan();
  /*
  OK文字列の受信待ち
  [引数]
   なし
  [戻り値]
   0: OKを受信した
   -1: OKを受信せず、タイムアウトした
  */
  int waitOk();
  /*
  IPV6アドレス取得
  [引数]
   なし
  [戻り値]
   void
  [副作用]
  ・IPV6アドレスが、addr配列に格納される。
  */
  void sendCmdAndWaitIpv6Address();
  /*
  コマンド送信
  [引数]
   コマンド文字列
  [戻り値]
   0: コマンド送信後、OKを受信した
   -1: コマンド送信後、OKを受信せずタイムアウトした
  */
  int sendCmdAndWaitOk(const char *cmd);
  /*
  受信バッファの解析
  [引数]
   バッファ
   バッファサイズ
  [戻り値]
   enum RCV_CODE
  */
  RCV_CODE parse(u_char *buf, size_t size);

public:
  inline long getPower() { return _power; };
  inline float getWattHourPlus() { return _powerPlus; };
  inline float getWattHourMinus() { return _powerMinus; };
  inline time_t getTimePlus() { return _timePlus; };
  inline time_t getTimeMinus() { return _timeMinus; };
  inline void setDebugView(DebugView *view) { _debugView = view; };
  inline CONNECT_STATE getConnectState() { return _connectState; };
  inline void setConnectState(CONNECT_STATE state) { _connectState = state; };
  inline char *getScnannedParam(int index) { return _para[index]; };
  /*
  初期化
  */
  void init();
  /*
  スマートメーターへの接続
  [引数]
   pwd: Bルート・パスワード
   bid: Bルート・ID
  [戻り値]
   void
  [副作用]
   ・addr配列に、ipv6アドレスが格納される
  */
  void connect(const char *pwd, const char *bid);
  /*
  スマートメータからの切断
  [引数]
   なし
  [戻り値]
   void
  */
  void disconnect();
  /*
  取得要求
  [戻り値]
   0: コマンド送信後、OKを受信した
   -1: コマンド送信後、OKを受信せずタイムアウトした
  */
  int request();
  /*
  受信待ち
  [引数]
   data: 受信データ格納バッファ（呼び出し側で確保すること）
   len: バッファサイズ
  [戻り値]
   enum RCV_CODE
  */
  RCV_CODE polling(char *data, size_t len);
};

#endif
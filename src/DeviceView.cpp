#include "DeviceView.h"
#include "PowerView.h"
#include "Node.h"

DeviceView *DeviceView::createView(Device *device, TFT_eSPI *lcd)
{
    uint16_t type = device->getClassType();
    if (type == 0x0288)
    {
        return new PowerView((SmartMeter *)device, lcd);
    }
    else if (type == 0x0130)
    {
        return new AirconView((Aircon *)device, lcd);
    }
    else if (type == 0x026b)
    {
        return new ElectricWaterHeaterView((ElectricWaterHeater *)device, lcd);
    }
    else if (type == 0x0279)
    {
        return new SolarPowerView((SolarPower *)device, lcd);
    }
    else if (type == 0x027d)
    {
        return new BatteryView((Battery *)device, lcd);
    }
    else if (type == 0x0011)
    {
        return new TempSensorView((TempSensor *)device, lcd);
    }
    else if (type == 0x0012)
    {
        return new HumiditySensorView((HumiditySensor *)device, lcd);
    }
    else if (type == 0x002d)
    {
        return new PressureSensorView((PressureSensor *)device, lcd);
    }
    else if (type == 0x001b)
    {
        return new CO2SensorView((CO2Sensor *)device, lcd);
    }
    else if (type == 0x001d)
    {
        return new VOCSensorView((VOCSensor *)device, lcd);
    }
    return nullptr;
}

//
DeviceView::DeviceView(Device *device, TFT_eSPI *lcd) : View(lcd)
{
    setDevice(device);
}
void DeviceView::init()
{
    getLcd()->setFont(&fonts::lgfxJapanGothic_24);
    int16_t fh = getLcd()->fontHeight();
    int32_t y = 16;
    getLcd()->fillRect(0, y, getLcd()->width(), getLcd()->height() - y * 2, TFT_BLACK);             // ヘッダ以外を消去
    getLcd()->drawRoundRect(0, y, getLcd()->width(), getLcd()->height() - y * 2 - 4, 5, TFT_WHITE); // 外枠
    _baseY = y + fh + 3;
    getLcd()->drawFastHLine(0, _baseY - 1, getLcd()->width());
}

//
int AirconView::_count = 0;
AirconView::AirconView(Aircon *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
{
    const int len = 6 + 2 + 1; // "Aircon" + "xx" + '\0'
    char *buf = (char *)malloc(len);
    snprintf(buf, len, "Aircon%02d", _count++);
    setName(buf);
}
void AirconView::init()
{
    DeviceView::init();
    getLcd()->setFont(&fonts::lgfxJapanGothic_24);
    getLcd()->setTextColor(TFT_WHITE);

    int x = getLcd()->width() / 2;
    int y = 16 + 1;
    getLcd()->setTextDatum(TC_DATUM);
    getLcd()->drawString("エアコン", x, y);

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    _fontHeight = getLcd()->fontHeight();
    getLcd()->setTextDatum(TL_DATUM);
    x = 2;
    y = _baseY;
    // getLcd()->drawString("識別番号", x, y);
    y += _fontHeight * 2;
    getLcd()->drawString("動作状態", x, y);
    y += _fontHeight;
    getLcd()->drawString("動作モード", x, y);
    y += _fontHeight;
    getLcd()->drawString("消費電力", x, y);
    y += _fontHeight;
    getLcd()->drawString("室温", x, y);
    y += _fontHeight;
    getLcd()->drawString("外気温", x, y);

    getLcd()->setTextDatum(TR_DATUM);
    x = getLcd()->width() - 2;
    y = _baseY;
    y += _fontHeight * 4;
    getLcd()->drawString("Ｗ", x, y);
    y += _fontHeight;
    getLcd()->drawString("℃", x, y);
    y += _fontHeight;
    getLcd()->drawString("℃", x, y);

    _fontWidth = getLcd()->textWidth("0");
}
void AirconView::update()
{
    char buf[64];
    Aircon *ac = (Aircon *)getDevice();

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    getLcd()->setTextColor(TFT_YELLOW);
    getLcd()->setTextDatum(TR_DATUM);

    int x = getLcd()->width() - 1 - _fontWidth * 3;

    int backColor = TFT_BLACK;
    int w = _fontWidth * 17 * 2;

    int y = _baseY;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(ac->getNode()->getId(), x, y);

    w = _fontWidth * 3 * 2;
    y += _fontHeight * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(ac->getOn(), x, y);

    w = _fontWidth * 3 * 2;
    y += _fontHeight;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(ac->getMode(), x, y);

    snprintf(buf, sizeof(buf), "%5d", ac->getPower());
    w = _fontWidth * 5;
    y += _fontHeight;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);

    int8_t tr = ac->getTempRoom();
    if (tr > 125)
        snprintf(buf, sizeof(buf), "不明");
    else
        snprintf(buf, sizeof(buf), "%3d", tr);
    w = _fontWidth * 4;
    y += _fontHeight;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);

    int8_t to = ac->getTempOut();
    if (to > 125)
        snprintf(buf, sizeof(buf), "不明");
    else
        snprintf(buf, sizeof(buf), "%3d", to);
    w = _fontWidth * 4;
    y += _fontHeight;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);
}

//
ElectricWaterHeaterView::ElectricWaterHeaterView(ElectricWaterHeater *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
{
    setName("Ecocute");
}
void ElectricWaterHeaterView::init()
{
    DeviceView::init();
    getLcd()->setFont(&fonts::lgfxJapanGothic_24);
    getLcd()->setTextColor(TFT_WHITE);

    int x = getLcd()->width() / 2;
    int y = 16 + 1;
    getLcd()->setTextDatum(TC_DATUM);
    getLcd()->drawString("エコキュート", x, y);

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    _fontHeight = getLcd()->fontHeight();
    getLcd()->setTextDatum(TL_DATUM);
    x = 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("湧き上げ状態", x, y);
    y += _fontHeight;
    getLcd()->drawString("風呂状態", x, y);
    y += _fontHeight;
    getLcd()->drawString("消費電力", x, y);
    y += _fontHeight;
    getLcd()->drawString("残湯量", x, y);

    getLcd()->setTextDatum(TR_DATUM);
    x = getLcd()->width() - 2;
    y = _baseY;
    y += _fontHeight * 4;
    getLcd()->drawString("Ｗ", x, y);
    y += _fontHeight;
    getLcd()->drawString("Ｌ", x, y);

    _fontWidth = getLcd()->textWidth("0");
}
void ElectricWaterHeaterView::update()
{
    char buf[64];
    ElectricWaterHeater *ewh = (ElectricWaterHeater *)getDevice();

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    getLcd()->setTextColor(TFT_YELLOW);
    getLcd()->setTextDatum(TR_DATUM);

    int x = getLcd()->width() - 1 - _fontWidth * 3;
    int y = _baseY;

    int backColor = TFT_BLACK;
    int w = _fontWidth * 17 * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(ewh->getNode()->getId(), x, y);

    w = _fontWidth * 3;
    y += _fontHeight * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(ewh->getBoil(), x, y);

    w = _fontWidth * 4 * 2;
    y += _fontHeight;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(ewh->getState(), x, y);

    snprintf(buf, sizeof(buf), "%5d", ewh->getPower());
    w = _fontWidth * 5;
    y += _fontHeight;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);

    snprintf(buf, sizeof(buf), "%5d", ewh->getTank());
    w = _fontWidth * 5;
    y += _fontHeight;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);
}

//
SolarPowerView::SolarPowerView(SolarPower *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
{
    setName("Solar");
}
void SolarPowerView::init()
{
    DeviceView::init();
    getLcd()->setFont(&fonts::lgfxJapanGothic_24);
    getLcd()->setTextColor(TFT_WHITE);

    int x = getLcd()->width() / 2;
    int y = 16 + 1;
    getLcd()->setTextDatum(TC_DATUM);
    getLcd()->drawString("太陽光発電", x, y);

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    _fontHeight = getLcd()->fontHeight();
    getLcd()->setTextDatum(TL_DATUM);
    x = 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("瞬時発電電力", x, y);
    y += _fontHeight;
    getLcd()->drawString("積算発電電力量", x, y);

    getLcd()->setTextDatum(TR_DATUM);
    x = getLcd()->width() - 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("Ｗ", x, y);
    y += _fontHeight;
    getLcd()->drawString("kWh", x, y);

    _fontWidth = getLcd()->textWidth("0");
}
void SolarPowerView::update()
{
    char buf[64];
    SolarPower *sp = (SolarPower *)getDevice();

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    getLcd()->setTextColor(TFT_YELLOW);
    getLcd()->setTextDatum(TR_DATUM);

    int x = getLcd()->width() - 1 - _fontWidth * 4;
    int y = _baseY;

    int backColor = TFT_BLACK;
    int w = _fontWidth * 17 * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(sp->getNode()->getId(), x, y);

    w = _fontWidth * 5;
    y += _fontHeight * 2;
    snprintf(buf, sizeof(buf), "%5d", sp->getPower());
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);

    snprintf(buf, sizeof(buf), "%11.3f", sp->getWattHour());
    w = _fontWidth * 11;
    y += _fontHeight;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);
}

//
BatteryView::BatteryView(Battery *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
{
    setName("Battery");
}
void BatteryView::init()
{
    DeviceView::init();
    getLcd()->setFont(&fonts::lgfxJapanGothic_24);
    getLcd()->setTextColor(TFT_WHITE);

    int x = getLcd()->width() / 2;
    int y = 16 + 1;
    getLcd()->setTextDatum(TC_DATUM);
    getLcd()->drawString("蓄電池", x, y);

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    _fontHeight = getLcd()->fontHeight();
    getLcd()->setTextDatum(TL_DATUM);
    x = 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("動作状態", x, y);
    y += _fontHeight;
    getLcd()->drawString("蓄電残量", x, y);

    getLcd()->setTextDatum(TR_DATUM);
    x = getLcd()->width() - 2;
    y = _baseY;
    y += _fontHeight * 3;
    getLcd()->drawString("％", x, y);

    _fontWidth = getLcd()->textWidth("0");
}
void BatteryView::update()
{
    char buf[64];
    Battery *batt = (Battery *)getDevice();

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    getLcd()->setTextColor(TFT_YELLOW);
    getLcd()->setTextDatum(TR_DATUM);

    int x = getLcd()->width() - 1 - _fontWidth * 3;
    int y = _baseY;

    int backColor = TFT_BLACK;
    int w = _fontWidth * 17 * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(batt->getNode()->getId(), x, y);

    w = _fontWidth * 9 * 2;
    y += _fontHeight * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(batt->getState(), x, y);

    snprintf(buf, sizeof(buf), "%3d", batt->getPercent());
    w = _fontWidth * 3;
    y += _fontHeight;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);
}

//
int TempSensorView::_count = 0;
TempSensorView::TempSensorView(TempSensor *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
{
    const int len = 5 + 2 + 1; // "Temp_" + "xx" + '\0'
    char *buf = (char *)malloc(len);
    snprintf(buf, len, "Temp_%02d", _count++);
    setName(buf);
}
void TempSensorView::init()
{
    DeviceView::init();
    getLcd()->setFont(&fonts::lgfxJapanGothic_24);
    getLcd()->setTextColor(TFT_WHITE);

    int x = getLcd()->width() / 2;
    int y = 16 + 1;
    getLcd()->setTextDatum(TC_DATUM);
    getLcd()->drawString("温度センサ", x, y);

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    _fontHeight = getLcd()->fontHeight();
    getLcd()->setTextDatum(TL_DATUM);
    x = 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("温度", x, y);

    getLcd()->setTextDatum(TR_DATUM);
    x = getLcd()->width() - 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("℃", x, y);

    _fontWidth = getLcd()->textWidth("0");
}
void TempSensorView::update()
{
    char buf[64];
    TempSensor *ts = (TempSensor *)getDevice();

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    getLcd()->setTextColor(TFT_YELLOW);
    getLcd()->setTextDatum(TR_DATUM);

    int x = getLcd()->width() - 1 - _fontWidth * 3;
    int y = _baseY;

    int backColor = TFT_BLACK;
    int w = _fontWidth * 17 * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(ts->getNode()->getId(), x, y);

    w = _fontWidth * 9 * 2;
    y += _fontHeight * 2;
    snprintf(buf, sizeof(buf), "%6.1f", ts->getValue());
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);
}

//
int HumiditySensorView::_count = 0;
HumiditySensorView::HumiditySensorView(HumiditySensor *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
{
    const int len = 5 + 2 + 1; // "Humi_" + "xx" + '\0'
    char *buf = (char *)malloc(len);
    snprintf(buf, len, "Humi_%02d", _count++);
    setName(buf);
}
void HumiditySensorView::init()
{
    DeviceView::init();
    getLcd()->setFont(&fonts::lgfxJapanGothic_24);
    getLcd()->setTextColor(TFT_WHITE);

    int x = getLcd()->width() / 2;
    int y = 16 + 1;
    getLcd()->setTextDatum(TC_DATUM);
    getLcd()->drawString("湿度センサ", x, y);

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    _fontHeight = getLcd()->fontHeight();
    getLcd()->setTextDatum(TL_DATUM);
    x = 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("湿度", x, y);

    getLcd()->setTextDatum(TR_DATUM);
    x = getLcd()->width() - 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("％", x, y);

    _fontWidth = getLcd()->textWidth("0");
}
void HumiditySensorView::update()
{
    char buf[64];
    HumiditySensor *hs = (HumiditySensor *)getDevice();

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    getLcd()->setTextColor(TFT_YELLOW);
    getLcd()->setTextDatum(TR_DATUM);

    int x = getLcd()->width() - 1 - _fontWidth * 3;
    int y = _baseY;

    int backColor = TFT_BLACK;
    int w = _fontWidth * 17 * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(hs->getNode()->getId(), x, y);

    w = _fontWidth * 9 * 2;
    y += _fontHeight * 2;
    snprintf(buf, sizeof(buf), "%3d", hs->getValue());
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);
}

//
int PressureSensorView::_count = 0;
PressureSensorView::PressureSensorView(PressureSensor *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
{
    const int len = 6 + 2 + 1; // "Press_" + "xx" + '\0'
    char *buf = (char *)malloc(len);
    snprintf(buf, len, "Press_%02d", _count++);
    setName(buf);
}
void PressureSensorView::init()
{
    DeviceView::init();
    getLcd()->setFont(&fonts::lgfxJapanGothic_24);
    getLcd()->setTextColor(TFT_WHITE);

    int x = getLcd()->width() / 2;
    int y = 16 + 1;
    getLcd()->setTextDatum(TC_DATUM);
    getLcd()->drawString("気圧センサ", x, y);

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    _fontHeight = getLcd()->fontHeight();
    getLcd()->setTextDatum(TL_DATUM);
    x = 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("気圧", x, y);

    getLcd()->setTextDatum(TR_DATUM);
    x = getLcd()->width() - 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("hPa", x, y);

    _fontWidth = getLcd()->textWidth("0");
}
void PressureSensorView::update()
{
    char buf[64];
    PressureSensor *ps = (PressureSensor *)getDevice();

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    getLcd()->setTextColor(TFT_YELLOW);
    getLcd()->setTextDatum(TR_DATUM);

    int x = getLcd()->width() - 1 - _fontWidth * 4;
    int y = _baseY;

    int backColor = TFT_BLACK;
    int w = _fontWidth * 17 * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(ps->getNode()->getId(), x, y);

    w = _fontWidth * 6;
    y += _fontHeight * 2;
    snprintf(buf, sizeof(buf), "%6.1f", ps->getValue());
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);
}

//
int CO2SensorView::_count = 0;
CO2SensorView::CO2SensorView(CO2Sensor *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
{
    const int len = 4 + 2 + 1; // "CO2_" + "xx" + '\0'
    char *buf = (char *)malloc(len);
    snprintf(buf, len, "CO2_%02d", _count++);
    setName(buf);
}
void CO2SensorView::init()
{
    DeviceView::init();
    getLcd()->setFont(&fonts::lgfxJapanGothic_24);
    getLcd()->setTextColor(TFT_WHITE);

    int x = getLcd()->width() / 2;
    int y = 16 + 1;
    getLcd()->setTextDatum(TC_DATUM);
    getLcd()->drawString("CO2センサ", x, y);

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    _fontHeight = getLcd()->fontHeight();
    getLcd()->setTextDatum(TL_DATUM);
    x = 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("CO2濃度", x, y);

    getLcd()->setTextDatum(TR_DATUM);
    x = getLcd()->width() - 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("ppm", x, y);

    _fontWidth = getLcd()->textWidth("0");
}
void CO2SensorView::update()
{
    char buf[64];
    CO2Sensor *cs = (CO2Sensor *)getDevice();

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    getLcd()->setTextColor(TFT_YELLOW);
    getLcd()->setTextDatum(TR_DATUM);

    int x = getLcd()->width() - 1 - _fontWidth * 4;
    int y = _baseY;

    int backColor = TFT_BLACK;
    int w = _fontWidth * 17 * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(cs->getNode()->getId(), x, y);

    w = _fontWidth * 5;
    y += _fontHeight * 2;
    snprintf(buf, sizeof(buf), "%5d", cs->getValue());
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);
}

//
int VOCSensorView::_count = 0;
VOCSensorView::VOCSensorView(VOCSensor *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
{
    const int len = 4 + 2 + 1; // "VOC_" + "xx" + '\0'
    char *buf = (char *)malloc(len);
    snprintf(buf, len, "VOC_%02d", _count++);
    setName(buf);
}
void VOCSensorView::init()
{
    DeviceView::init();
    getLcd()->setFont(&fonts::lgfxJapanGothic_24);
    getLcd()->setTextColor(TFT_WHITE);

    int x = getLcd()->width() / 2;
    int y = 16 + 1;
    getLcd()->setTextDatum(TC_DATUM);
    getLcd()->drawString("VOCセンサ", x, y);

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    _fontHeight = getLcd()->fontHeight();
    getLcd()->setTextDatum(TL_DATUM);
    x = 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("VOC濃度", x, y);

    getLcd()->setTextDatum(TR_DATUM);
    x = getLcd()->width() - 2;
    y = _baseY;
    y += _fontHeight * 2;
    getLcd()->drawString("ppb", x, y);

    _fontWidth = getLcd()->textWidth("0");
}
void VOCSensorView::update()
{
    char buf[64];
    VOCSensor *cs = (VOCSensor *)getDevice();

    getLcd()->setFont(&fonts::lgfxJapanGothic_16);
    getLcd()->setTextColor(TFT_YELLOW);
    getLcd()->setTextDatum(TR_DATUM);

    int x = getLcd()->width() - 1 - _fontWidth * 4;
    int y = _baseY;

    int backColor = TFT_BLACK;
    int w = _fontWidth * 17 * 2;
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(cs->getNode()->getId(), x, y);

    w = _fontWidth * 8;
    y += _fontHeight * 2;
    snprintf(buf, sizeof(buf), "%8ld", cs->getLValue());
    getLcd()->fillRect(x - w, y, w, _fontHeight, backColor);
    getLcd()->drawString(buf, x, y);
}

#ifndef _DEVICEVIEW_H_
#define _DEVICEVIEW_H_
#include "View.h"
#include "Device.h"

class DeviceView : public View
{
private:
    Device *_device;
    char *_name;

protected:
    int16_t _baseY;
    // 1文字高
    int16_t _fontHeight;
    // 1文字幅
    int16_t _fontWidth;

public:
    DeviceView(Device *device, TFT_eSPI *lcd);
    inline void setDevice(Device *device) { _device = device; }
    inline void setName(char *name) { _name = name; }
    inline char *getName() { return _name; }
    virtual inline Device *getDevice() { return _device; }
    virtual void init();
    virtual void update();
    static DeviceView *createView(Device *device, TFT_eSPI *lcd);
};

class AirconView : public DeviceView
{
private:
    static int _count;

public:
    AirconView(Aircon *device, TFT_eSPI *lcd);
    virtual void init();
    virtual void update();
};

class ElectricWaterHeaterView : public DeviceView
{
public:
    ElectricWaterHeaterView(ElectricWaterHeater *device, TFT_eSPI *lcd);
    virtual void init();
    virtual void update();
};

class SolarPowerView : public DeviceView
{
public:
    SolarPowerView(SolarPower *device, TFT_eSPI *lcd);
    virtual void init();
    virtual void update();
};

class BatteryView : public DeviceView
{
public:
    BatteryView(Battery *device, TFT_eSPI *lcd);
    virtual void init();
    virtual void update();
};

class TempSensorView : public DeviceView
{
private:
    static int _count;

public:
    TempSensorView(TempSensor *device, TFT_eSPI *lcd);
    virtual void init();
    virtual void update();
};

class HumiditySensorView : public DeviceView
{
private:
    static int _count;

public:
    HumiditySensorView(HumiditySensor *device, TFT_eSPI *lcd);
    virtual void init();
    virtual void update();
};

class PressureSensorView : public DeviceView
{
private:
    static int _count;

public:
    PressureSensorView(PressureSensor *device, TFT_eSPI *lcd);
    virtual void init();
    virtual void update();
};

class CO2SensorView : public DeviceView
{
private:
    static int _count;

public:
    CO2SensorView(CO2Sensor *device, TFT_eSPI *lcd);
    virtual void init();
    virtual void update();
};

class VOCSensorView : public DeviceView
{
private:
    static int _count;

public:
    VOCSensorView(VOCSensor *device, TFT_eSPI *lcd);
    virtual void init();
    virtual void update();
};

#endif
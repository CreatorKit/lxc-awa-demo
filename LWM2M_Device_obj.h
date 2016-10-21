#ifndef LWM2M_DEVICE_OBJ_H
#define LWM2M_DEVICE_OBJ_H

#define DEVICE_OBJECT_ID        "3"
#define MANUFACTURER            "/3/0/0"
#define DEVICE_TYPE             "/3/0/17"
#define MODEL_NUMBER            "/3/0/1"
#define SERIAL_NUMBER           "/3/0/2"
#define HARDWARE_VERSION        "/3/0/18"
#define FIRMWARE_VERSION        "/3/0/3"
#define SOFTWARE_VERSION        "/3/0/19"
#define AVAILABLE_POWER_SOURCES "/3/0/6"
#define POWER_SOURCE_VOLTAGE    "/3/0/7"
#define POWER_SOURCE_CURRENT    "/3/0/8"
#define BATTERY_LEVEL           "/3/0/9"
#define REBOOT                  "/3/0/4"
#define CURRENT_TIME            "/3/0/13"
#define UTCOFFSET               "/3/0/14"
#define TIMEZONE                "/3/0/15"
#define ERROR_CODE              "/3/0/11"
#define RESET_ERROR_CODE        "/3/0/12"
#define SUPPORTED_BINGING_AND_MODES "/3/0/16"

#define OBJECT_INSTANCE(obj, inst) "/" obj "/" #inst

typedef enum{
  INSTANCE_0 = 0,
  INSTANCE_1,
  INSTANCE_2,
  INSTANCE_3,
}InstanceID;

typedef enum{
  DC_POWER = 0,
  INTERNAL_BATTERY,
  EXTERNAL_BATTERY,
  POWER_OVER_ETHERNET,
  USB,
  AC_MAIN_POWER,
  SOLAR,
}AvailablePowerSources;

#define DEVICE_MAX_BUFF		100
#define OPERATION_PERFORM_TIMEOUT 1000

typedef struct{
    char Manufacturer[DEVICE_MAX_BUFF];
    char DeviceType[DEVICE_MAX_BUFF];
    char ModelNumber[DEVICE_MAX_BUFF];
    char SerialNumber[DEVICE_MAX_BUFF];
    char HardwareVersion[DEVICE_MAX_BUFF];
    char FirmwareVersion[DEVICE_MAX_BUFF];
    char SoftwareVersion[DEVICE_MAX_BUFF];
    int ErrorCode;
    int PowerSourceVoltage;
    int PowerSourceCurrent;
    int BatteryLevel;
}_Device_obj;

_Device_obj DeviceObj;


void InitDevice(AwaClientSession * session);
void DeviceControl(AwaClientSession * session);

#endif
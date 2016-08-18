#include <stdio.h>
#include <string.h>
#include <time.h>

#include <awa/common.h>
#include <awa/client.h>

#include "LWM2M_Device_obj.h"
#include <inttypes.h>
/*
 * This function will be called by the API when the client sends
 * a notification of execution. When invoked, arguments provides information
 * about the execute arguments supplied by the server, and context
 * is the supplied pointer to any application data.
 */
static void RebootexecuteCallback(const AwaExecuteArguments * arguments, void * context)
{
    const char * userData = (const char *)context;
    printf("Callback received. Context = %s\n", userData);

    printf("Resource executed [%zu bytes payload]\n", arguments->Size);

    printf("REBOOT !!!!!\n");
    system("reboot");

    exit(0);
}

static void changeCallback(const AwaChangeSet * changeSet, void * context)
{
    const char * userData = (const char *)context;
    printf("Callback received. Context = %s\n", userData);

    const char * value;
    const AwaTime * tvalue;
    if(!strcmp(userData,TIMEZONE))
    {
      AwaChangeSet_GetValueAsCStringPointer(changeSet, TIMEZONE, &value);
      printf("Value of resource %s changed to: %s\n", TIMEZONE,value);
    }else if(!strcmp(userData,UTCOFFSET))
    {
      AwaChangeSet_GetValueAsCStringPointer(changeSet, UTCOFFSET, &value);
      printf("Value of resource %s changed to: %s\n", UTCOFFSET,value);
    }else if(!strcmp(userData,CURRENT_TIME))
    {
      AwaChangeSet_GetValueAsTimePointer(changeSet, CURRENT_TIME, &tvalue);
      printf("Value of resource %s changed to: %ld\n", CURRENT_TIME,(long int)tvalue);
    }
}

static void UpdateManufacturerName(AwaClientSession * session)
{
    /* Create SET operation */
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    /*
     * This example uses resource /3/0/0 which is the Manufacturer
     * resource in the standard Device object. It is a string.
     */

    /* Provide a path and value for the resource */
    AwaClientSetOperation_AddValueAsCString(operation, MANUFACTURER, DeviceObj.Manufacturer);

    /* Perform the SET operation */
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* Operations must be freed after use */
    AwaClientSetOperation_Free(&operation);
}

static void UpdateDeviceType(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_AddValueAsCString(operation, DEVICE_TYPE, DeviceObj.DeviceType);

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    AwaClientSetOperation_Free(&operation);
}

static void UpdateModelNumber(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_AddValueAsCString(operation, MODEL_NUMBER, DeviceObj.ModelNumber);

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    AwaClientSetOperation_Free(&operation);
}

static void UpdateSerialNumber(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_AddValueAsCString(operation, SERIAL_NUMBER, DeviceObj.SerialNumber);

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    AwaClientSetOperation_Free(&operation);
}

static void UpdateHardwareVersion(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_AddValueAsCString(operation, HARDWARE_VERSION, DeviceObj.HardwareVersion);

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    AwaClientSetOperation_Free(&operation);
}

static void UpdateFirmwareVersion(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_AddValueAsCString(operation, FIRMWARE_VERSION, DeviceObj.FirmwareVersion);

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    AwaClientSetOperation_Free(&operation);
}

static void UpdateSoftwareVersion(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_AddValueAsCString(operation, SOFTWARE_VERSION, DeviceObj.SoftwareVersion);

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    AwaClientSetOperation_Free(&operation);
}

static void Reboot(AwaClientSession * session)
{
    /* Application-specific data */
    const char * userData = REBOOT;

    /*
    * This example uses resource /3/0/4 which is the Reboot
    * resource in the standard Device object. It is an executable (None type) resource.
    */

    /*
    * Create a new execute subscription to resource /3/0/4.
    * Data can be provided to the callback function via the context pointer.
    */
    //AwaClientExecuteSubscription
    // tens que arranjar outra forma para fazer isto
    AwaClientExecuteSubscription * subscription = AwaClientExecuteSubscription_New(REBOOT, RebootexecuteCallback, (void*)userData);

    /* Start listening to notifications */
    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(session);
    AwaClientSubscribeOperation_AddExecuteSubscription(subscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(subscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&subscribeOperation);
}

static void FactoryReset(AwaClientSession * session)
{
}

static void UpdateAvailablePowerSources(AwaClientSession * session)
{
    //this resource got multiple instances
    AwaIntegerArray * power_source1 = AwaIntegerArray_New();
    //create instace 0 valeu 7
    AwaIntegerArray_SetValue(power_source1, INSTANCE_0, INTERNAL_BATTERY);
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);
    AwaClientSetOperation_AddValueAsIntegerArray(operation, AVAILABLE_POWER_SOURCES, power_source1);
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

static void UpdatePowerSourceVoltage(AwaClientSession * session)
{
    //this resource got multiple instances
    AwaIntegerArray * source_array = AwaIntegerArray_New();
    //create instace 0 and voltage value
    AwaIntegerArray_SetValue(source_array, INSTANCE_0, DeviceObj.PowerSourceVoltage);
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);
    AwaClientSetOperation_AddValueAsIntegerArray(operation, POWER_SOURCE_VOLTAGE, source_array);
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

static void UpdatePowerSourceCurrent(AwaClientSession * session)
{
    //this resource got multiple instances
    AwaIntegerArray * source_array = AwaIntegerArray_New();
    //create instace 0 and voltage value
    AwaIntegerArray_SetValue(source_array, INSTANCE_0, DeviceObj.PowerSourceCurrent);
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);
    AwaClientSetOperation_AddValueAsIntegerArray(operation, POWER_SOURCE_CURRENT, source_array);
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

static void UpdateBatteryLevel(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);
    AwaClientSetOperation_AddValueAsInteger(operation, BATTERY_LEVEL, DeviceObj.BatteryLevel);
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

void UpdateTimezone(AwaClientSession * session)
{
}

static void Timezone(AwaClientSession * session)
{
    /* Application-specific data */
    const char * userData = TIMEZONE;

    time_t t = time(NULL);
    struct tm lt = {0};
    localtime_r(&t, &lt);
    printf("The time zone is '%s'.\n", lt.tm_zone);

    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);
    AwaClientSetOperation_AddValueAsCString(operation, TIMEZONE, lt.tm_zone);
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);


    AwaClientChangeSubscription *subscription = AwaClientChangeSubscription_New(TIMEZONE, changeCallback, (void*)userData);
    /* Start listening to notifications */
    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(session);
    AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(subscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&subscribeOperation);
}

void UpdateUTCOffset(AwaClientSession * session)
{
}

static void UTCOffset(AwaClientSession * session)
{
    /* Application-specific data */
    const char * userData = UTCOFFSET;
    char str[DEVICE_MAX_BUFF];

    time_t t = time(NULL);
    struct tm lt = {0};
    localtime_r(&t, &lt);
    printf("Offset to GMT is %lds.\n", lt.tm_gmtoff);
    sprintf(str,"%lds",lt.tm_gmtoff);

    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);
    AwaClientSetOperation_AddValueAsCString(operation, UTCOFFSET, str);
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);

    AwaClientChangeSubscription *subscription = AwaClientChangeSubscription_New(UTCOFFSET, changeCallback, (void*)userData);
    /* Start listening to notifications */
    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(session);
    AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(subscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&subscribeOperation);
}

/*see timers.c to set CurrentTime*/
void UpdateCurrentTime(AwaClientSession * session)
{
}

/*see timers.c to get CurrentTime*/
static void CurrentTime(AwaClientSession * session)
{
    /* Application-specific data */
    const char * userData = CURRENT_TIME;
    char str[DEVICE_MAX_BUFF];

    time_t t = time(NULL);
    struct tm lt = {0};
    localtime_r(&t, &lt);
    printf("System %ld\n", time(&t));
    sprintf(str,"%ld",time(&t));

    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);
    AwaClientSetOperation_AddValueAsTime(operation, CURRENT_TIME, time(&t));
    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);

    AwaClientChangeSubscription *subscription = AwaClientChangeSubscription_New(CURRENT_TIME, changeCallback, (void*)userData);
    /* Start listening to notifications */
    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(session);
    AwaClientSubscribeOperation_AddChangeSubscription(subscribeOperation, subscription);
    AwaClientSubscribeOperation_Perform(subscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&subscribeOperation);
}

static void UpdateErrorCode(AwaClientSession * session)
{
    //this resource got multiple instances
    AwaIntegerArray * ErrorcodeArray = AwaIntegerArray_New();
    //create instace 0 value DeviceObj.ErrorCode
    AwaIntegerArray_SetValue(ErrorcodeArray, 0, DeviceObj.ErrorCode);


    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_AddValueAsIntegerArray(operation, ERROR_CODE, ErrorcodeArray);

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    AwaClientSetOperation_Free(&operation);
}

void InitDevice(AwaClientSession * session)
{
    strcpy(DeviceObj.Manufacturer,"Semiotics");
    strcpy(DeviceObj.DeviceType,"Stock");
    strcpy(DeviceObj.ModelNumber,"1");
    strcpy(DeviceObj.SerialNumber,"12345");
    strcpy(DeviceObj.HardwareVersion,"V0.1");
    strcpy(DeviceObj.FirmwareVersion,"V0.0.1");
    strcpy(DeviceObj.SoftwareVersion,"openWRT Vxx");
    DeviceObj.ErrorCode = 0;

    UpdateManufacturerName(session);
    UpdateDeviceType(session);
    UpdateModelNumber(session);
    UpdateSerialNumber(session);
    UpdateHardwareVersion(session);
    UpdateFirmwareVersion(session);
    Reboot(session);
    FactoryReset(session);
    UpdateSoftwareVersion(session);
    UpdateAvailablePowerSources(session);
    UpdatePowerSourceVoltage(session);
    UpdatePowerSourceCurrent(session);
    UpdateBatteryLevel(session);
    Timezone(session);
    UTCOffset(session);
    CurrentTime(session);
    UpdateErrorCode(session);
}

void DeviceControl(AwaClientSession * session)
{
  UpdatePowerSourceVoltage(session);
  UpdatePowerSourceCurrent(session);
  UpdateBatteryLevel(session);
}

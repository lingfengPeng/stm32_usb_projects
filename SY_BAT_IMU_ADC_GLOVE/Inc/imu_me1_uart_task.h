#ifndef __IMU_ME1_UART_TASK_H__
#define __IMU_ME1_UART_TASK_H__

#include <stdlib.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "misc.h"
#include "adc.h"

#include "uart-API.h"
#include "uart-line-IO.h"
int init_imu_me1_uart_task(void);
void UART3_RxIdleCallback(UART_HandleTypeDef *huart);

#define MAX_PACKET_DATA_LENGTH 128
typedef struct _LpmsModbusPacket
{
    uint8_t start;
    uint16_t address;
    uint16_t function;
    uint16_t length;
    uint8_t data[MAX_PACKET_DATA_LENGTH];
    uint16_t lrcCheck;
    uint16_t end;
} LpmsPacket;

typedef struct
{
    float time;
    float gyr[3];
    float acc[3];
    float mag[3];
    float angular[3];
    float quat[4];
    float euler[3];
    float linAcc[3];
    float temp;
} LpmsData_t;

typedef struct
{
    uint16_t Id;
    uint32_t Config;
    uint32_t dataType;
    uint32_t Status;
    uint32_t StreamFreq;
    uint32_t Com_Mode;
    void *pData;
} LpmsHandle_t;

typedef union
{
    uint8_t u8vals[4];
    uint32_t u32val;
    float fval;
} f2int_t,int2f_t;

#define R_TO_D    (57.295780f)//Radian to Degree
#endif


#ifndef __HX711_H
#define __HX711_H

#include "stm32f1xx_hal.h"

#define HX711_SCK(x)	(x) ? HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET)
#define HX711_DOUT	HAL_GPIO_ReadPin(DOUT_GPIO_Port, DOUT_Pin)


extern uint32_t HX711_Read(void);
extern void Get_Maopi(void);
extern void Get_Weight(void);

extern uint32_t HX711_Buffer;
extern uint32_t Weight_Maopi;
extern int32_t Weight_Shiwu;
extern uint8_t Flag_Error;

#endif


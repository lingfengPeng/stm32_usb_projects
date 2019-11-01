#include "ble_wt-bt200a_uart_task.h"
/*****************************************************************************************
ble reset and glove restart task

*****************************************************************************************/
void start_ble_reset_task(void const *argument);// thread function
osThreadId ble_reset_taskHandle;// thread id
osThreadDef(ble_reset_task, start_ble_reset_task, osPriorityNormal, 0, 128);// thread object

int init_ble_reset_task(void){
	
  ble_reset_taskHandle = osThreadCreate (osThread(ble_reset_task), NULL);
  if (!ble_reset_taskHandle) return(-1);
  
  return(0);
}

uint8_t is_ble_connected = 4;
uint32_t ble_connect_mark = 5000;
void start_ble_reset_task(void const *argument)
{
	HAL_GPIO_WritePin(BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_SET);
	osDelay(5000);
	while(1){
		if(is_ble_connected){
			if((HAL_GetTick() - ble_connect_mark) > 120000){
				ble_connect_mark = HAL_GetTick();
				is_ble_connected--;
				if(is_ble_connected == 1){
					HAL_GPIO_WritePin(PWR_CTL_GPIO_Port, PWR_CTL_Pin, GPIO_PIN_RESET);
					osDelay(osWaitForever);
				}
				HAL_GPIO_WritePin(BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_RESET);
				osDelay(200);
				HAL_GPIO_WritePin(BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_SET);
				osDelay(200);
			}
			if(!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)){
				is_ble_connected = 0;
			}
		}else if(is_ble_connected == 0){
			if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)){
				HAL_GPIO_WritePin(BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_RESET);
				osDelay(200);
				HAL_GPIO_WritePin(BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_SET);		
				osDelay(200);
				ble_connect_mark = HAL_GetTick();
				is_ble_connected = 4;
			}
		}
		osDelay(100);
	}
}

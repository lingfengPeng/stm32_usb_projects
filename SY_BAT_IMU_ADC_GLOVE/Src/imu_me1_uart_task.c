#include "imu_me1_uart_task.h"
#include "ble_wt-bt200a_uart_task.h"
/*****************************************************************************************/
/*adc and valve ctrl thread*/
void start_imu_me1_uart_task(void const *argument);// thread function
osThreadId imu_me1_uart_taskHandle;// thread id
osThreadDef(imu_me1_uart_task, start_imu_me1_uart_task, osPriorityNormal, 0, 128);// thread object
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
float int2float(float *p_f, uint8_t *p_u8);


int init_imu_me1_uart_task(void){
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
  imu_me1_uart_taskHandle = osThreadCreate (osThread(imu_me1_uart_task), NULL);
  if (!imu_me1_uart_taskHandle) return(-1);
  
  return(0);
}

uint16_t imu_msg_len;
uint32_t imu_send_mark = 2000;
#define IMU_ME1_UART_RX_SIZE 128
uint8_t imu_me1_uart_RxBuf[IMU_ME1_UART_RX_SIZE];
uint8_t imu_test_cnt = 0;
uint8_t ble_test_buff[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
/*****************************************************************************************/

void start_imu_me1_uart_task (void const *argument) {

	HAL_UART_DMAStop(&huart3);
	HAL_UART_Receive_DMA(&huart3,imu_me1_uart_RxBuf,IMU_ME1_UART_RX_SIZE);	
	for(;;){
		osSignalWait(SIG_USER_1,osWaitForever);
		if((imu_msg_len == 0x1f)&&(imu_me1_uart_RxBuf[0] == 0x3a)){
			LpmsData_t siyi_imu_msg;
			int2float(&siyi_imu_msg.euler[0], &imu_me1_uart_RxBuf[11]);
			siyi_imu_msg.euler[0] *= R_TO_D;
			int2float(&siyi_imu_msg.euler[1], &imu_me1_uart_RxBuf[15]);
			siyi_imu_msg.euler[1] *= R_TO_D;
			int2float(&siyi_imu_msg.euler[2], &imu_me1_uart_RxBuf[19]);
			siyi_imu_msg.euler[2] *= R_TO_D;
			if(HAL_GetTick() - imu_send_mark > 33){
				imu_send_mark = HAL_GetTick();
				if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET){
					struct imu_msg_t *p = SerialDatagramEvtAlloc(sizeof(*p));
					if(p){
						extern uint16_t adc_transformed_resualt[5];
						SERIAL_DATAGRAM_INIT((*p), imu_msg);
						p->angle[0] = siyi_imu_msg.euler[0]*1000;
						p->angle[1] = siyi_imu_msg.euler[1]*1000;
						p->angle[2] = siyi_imu_msg.euler[2]*1000;
						memcpy(p->adc, adc_transformed_resualt, sizeof(adc_transformed_resualt));
						SerialDatagramEvtSend(p);
					}
				}
			}
		}
	}
}


extern DMA_HandleTypeDef hdma_usart3_rx;
void UART3_RxIdleCallback(UART_HandleTypeDef *huart)
{
	if(__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE)){
		__HAL_UART_CLEAR_IDLEFLAG(huart);
		
		imu_msg_len = IMU_ME1_UART_RX_SIZE - hdma_usart3_rx.Instance->CNDTR;
		osSignalSet(imu_me1_uart_taskHandle,SIG_USER_1);
		HAL_UART_DMAStop(huart);
		HAL_UART_Receive_DMA(huart,imu_me1_uart_RxBuf,IMU_ME1_UART_RX_SIZE);
	}
}

float int2float(float *p_f, uint8_t *p_u8)
{
	int2f_t i2f;
	for(uint8_t i=0;i<4;i++){
		i2f.u8vals[i] = p_u8[i];
	}
	*p_f = i2f.fval;
	return i2f.fval;
}

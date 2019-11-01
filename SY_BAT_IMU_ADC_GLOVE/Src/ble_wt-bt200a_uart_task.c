#include "ble_wt-bt200a_uart_task.h"

#define EVENT_LOOP_TIME_IN_MILLI_SECOND 10

osMailQDef(host_uart_tx, 30, uint8_t[MAX_SIZE_OF_SERIAL_DATAGRAM_EVENT]); 
static osMailQId mail_queue_id_for_host_tx;


DEFINE_MAIL_QUEUE_FOR_SERIAL_DATAGRAM_CMD(calibration_cmd,1);

uint16_t ble_msg_len;
#define BLE_UART_RX_SIZE 128
uint8_t ble_uart_RxBuf[BLE_UART_RX_SIZE];

#if 1
/*****************************************************************************************/
/*uart send to host thread*/
void thread_of_host_uart_tx (void const *argument); // thread function
osThreadId tid_thread_of_host_uart_tx;// thread id
osThreadDef(host_uart_tx, thread_of_host_uart_tx, osPriorityHigh, 0, 128);// thread object

/*uart recvive from host thread*/
void thread_of_host_uart_rx (void const *argument);// thread function
osThreadId tid_thread_of_host_uart_rx;// thread id
osThreadDef (host_uart_rx, thread_of_host_uart_rx, osPriorityHigh, 0, 128);// thread object
extern UART_HandleTypeDef huart2;

int init_thread_of_host_uart_tx (void) {
	
	mail_queue_id_for_host_tx = osMailCreate(osMailQ(host_uart_tx), NULL);

  tid_thread_of_host_uart_tx = osThreadCreate (osThread(host_uart_tx), NULL);
  if (!tid_thread_of_host_uart_tx)  return(-1);
  
  return(0);
}


int init_thread_of_host_uart_rx (void) {

	INIT_MAIL_QUEUE_FOR_SERIAL_DATAGRAM_CMD(calibration_cmd);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	
  tid_thread_of_host_uart_rx = osThreadCreate (osThread(host_uart_rx), NULL);
  if (!tid_thread_of_host_uart_rx)  return(-1);
  
  return(0);
}
/*****************************************************************************************/
#endif

//thread for host tx
uint32_t host_uart_tx_mark;
void thread_of_host_uart_tx (void const *argument) {
  for(;;){
		uint8_t len;
		//蓝牙会出现拼包的情况
		if(HAL_GetTick() - host_uart_tx_mark > 5){
			host_uart_tx_mark = HAL_GetTick();			
			osEvent evt = osMailGet(mail_queue_id_for_host_tx, osWaitForever);
			struct uart_head_t *head = evt.value.p;
			if (evt.status == osEventMail && head != NULL) {
				if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET){
					len = head->body_len;
					if (len <= MAX_SIZE_OF_SERIAL_DATAGRAM_EVENT) {
						host_uart_datagram_send(head, len);
					}
				}
			}
			osMailFree(mail_queue_id_for_host_tx, head);		
		}
  }
}

//thread for host rx
void thread_of_host_uart_rx (void const *argument) {
	HAL_UART_DMAStop(&huart2);
	HAL_UART_Receive_DMA(&huart2,ble_uart_RxBuf,BLE_UART_RX_SIZE);	
  for(;;){
		uint8_t buf[128];
		size_t buf_size;
		size_t skipped_count;
		for(;;){
			int ret = get_raw_datagram_from_serial(buf, sizeof buf, &buf_size, &skipped_count);
			if (!ret) {
				break;
			}
			ble_rev_process(buf, buf_size);
		}
  }
}


void ble_rev_process(const void *msg, size_t msg_len)
{
	const struct uart_head_t *head = msg;
	if(head->type == calibration_cmd){
		extern uint8_t is_calibration;
		const struct serial_calibration_cmd_t *p = msg;
		is_calibration = p->cmd;	
	}
}

void *SerialDatagramEvtAlloc(size_t size)
{
	void *ret = NULL;
	if (size <= MAX_SIZE_OF_SERIAL_DATAGRAM_EVENT) {
		ret = osMailAlloc(mail_queue_id_for_host_tx, 0);
	}
	if (!ret) {
		return NULL;
	}
	return ret;
}

int SerialDatagramEvtSend(void *ptr)
{
	osStatus status = osMailPut(mail_queue_id_for_host_tx, ptr);
	return status == osOK;
}

void SerialDatagramEvtFree(void *ptr)
{
	osMailFree(mail_queue_id_for_host_tx, ptr);
}

int host_uart_datagram_send(void *msg, const size_t msg_len)
{
	uint8_t buf[128];
	uint8_t *p = buf;
	int ret;	
	uint8_t *s = msg;
	memcpy(p, s, msg_len);
	*(p + msg_len) = sy_checkSum(s+2, msg_len -2);
	ret = send_raw_datagram_to_serial(buf, msg_len + 1);
	return ret;
}


int send_raw_datagram_to_serial(const void *raw_datagram, size_t raw_datagram_len)
{
	struct AsyncIoResult_t IoResult = { 0, osThreadGetId() };
	osStatus status = StartUartTx(2, raw_datagram, raw_datagram_len, NotifyAsyncIoFinished, &IoResult);
	if (status != osOK) {
		return 0;
	}
	osSignalWait(SIG_SERVER_FINISHED, osWaitForever);
	// TODO: Wait time should not be 'forever'. if it's out of time, should Call StopUartXX.
	return 1;
}

int get_raw_datagram_from_serial(uint8_t *raw_datagram, size_t max_size, size_t *actual_size_ptr, size_t *skipped_byte_count_ptr)
{
	struct AsyncIoResult_t IoResult = { 0, osThreadGetId() };
	osSignalWait(SIG_USER_1, osWaitForever);
	uint8_t *p = ble_uart_RxBuf;
	
	while(p < ble_uart_RxBuf + ble_msg_len){
		if((*p == 0xfe)&&(*(p+1) == 0xfe)){
			p += 2;
			if(sy_checkSum(p, *(p+1)) == *(p+*(p+1))){
				*actual_size_ptr = *(p+1)+3;
				memcpy(raw_datagram, p-2, *actual_size_ptr);
				return 1;
			}
		}else{
			p++;
		}
	}
	return 0;
}


extern DMA_HandleTypeDef hdma_usart2_rx;
void UART2_RxIdleCallback(UART_HandleTypeDef *huart)
{
	if(__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE)){
		__HAL_UART_CLEAR_IDLEFLAG(huart);
		
		ble_msg_len = BLE_UART_RX_SIZE - hdma_usart2_rx.Instance->CNDTR;
		osSignalSet(tid_thread_of_host_uart_rx,SIG_USER_1);
		HAL_UART_DMAStop(huart);
		HAL_UART_Receive_DMA(huart,ble_uart_RxBuf,BLE_UART_RX_SIZE);
	}
}

uint8_t sy_checkSum(uint8_t *content, uint8_t len){
   uint8_t result = 0;
   for (int i = 0; i < len; i++) {
      result +=content[i];

   }
   return  ~result;
}

/****************************************END OF FILE*************************************/

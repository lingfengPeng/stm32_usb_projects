#include "thread_ble_transfer.h"

osThreadId ble_transfer_handler;
osThreadId setup_data_frame_tx_handler;
osMailQId mailid_of_ble_tx;

uint8_t calibration_cmd_global = 1;

void init_thread_ble_transfer(void)
{
	osThreadDef(ble_transfer, thread_ble_transfer, osPriorityNormal, 0, 128);
	ble_transfer_handler = osThreadCreate(osThread(ble_transfer), NULL);
	
	osThreadDef(setup_data_frame_tx, thread_setup_data_frame_tx, osPriorityNormal, 0, 128);
	setup_data_frame_tx_handler = osThreadCreate(osThread(setup_data_frame_tx), NULL);
	
	osMailQDef(ble_tx, 10, uint8_t[128]);
	mailid_of_ble_tx = osMailCreate(osMailQ(ble_tx), NULL);
}

void thread_ble_transfer(void const * argument)
{
	for(;;)
	{
		osEvent evt = osMailGet(mailid_of_ble_tx, 0);
		
		if(evt.status == osEventMail && evt.value.p != NULL)
		{
			struct calibration_cmd_t *calibration_cmd = evt.value.p;
			
			HAL_UART_Transmit(&huart1, (uint8_t *)calibration_cmd, sizeof(struct calibration_cmd_t), 100);
			
			osMailFree(mailid_of_ble_tx, calibration_cmd);
		}
	}
}

void thread_setup_data_frame_tx(void const *argument)
{
	for(;;)
	{
		if(calibration_cmd_global != 2)
		{
			struct calibration_cmd_t *p = SerialDatagramEvtAlloc(sizeof (*p));
			if(p)
			{
				SERIAL_DATAGRAM_INIT((*p), calibration_cmd);
				p->cmd = calibration_cmd_global;
				SerialDatagramEvtSend(p);
//				calibration_cmd_global = 2;
			}
		}
		osDelay(50);
	}
}

void *SerialDatagramEvtAlloc(size_t size)
{
	void *ret = NULL;
	if (size <= MAX_SIZE_OF_SERIAL_DATAGRAM_EVENT) {
		ret = osMailAlloc(mailid_of_ble_tx, 0);
	}
	if (!ret) {
		return NULL;
	}
	return ret;
}

int SerialDatagramEvtSend(void *ptr)
{
	osStatus status = osMailPut(mailid_of_ble_tx, ptr);
	return status == osOK;
}


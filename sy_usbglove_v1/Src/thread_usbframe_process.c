#include "thread_usbframe_process.h"

osThreadId usbframe_process_handler;
osMailQId mailid_of_usb_tx;


transmit_state_t transfer_state = Transmit_Stop;
imu_angle_t last_angle;
usb_hid_state_t mouse_state;
usb_adc_state_t glove_state;
uint16_t adc_value[5];


extern osThreadId mouse_frame_handler;
extern osThreadId glove_frame_handler;


void init_thread_usbframe_process(void)
{
	osThreadDef(usbframe_process, thread_usbframe_process, osPriorityNormal, 0, 128);
	usbframe_process_handler = osThreadCreate(osThread(usbframe_process), NULL);
	
	osMailQDef(usb_frame_tx, 30, uint8_t[128]);
	mailid_of_usb_tx = osMailCreate(osMailQ(usb_frame_tx), NULL);
}

void thread_usbframe_process(void const * argument)
{
	for(;;)
	{
		while(UART_Frame.Rx_Flag == 1)
		{
			UART_Frame.Rx_Flag = 0;
			
			if(UART_Frame.Rx_Size != 0)
			{
				uint8_t buffer_temp[MAX_UART_BUF_LEN];
			
				memcpy(buffer_temp, UART_Frame.RxBuff, UART_Frame.Rx_Size);
				
				if(1)//(sy_checkSum(buffer_temp, UART_Frame.Rx_Size) == buffer_temp[UART_Frame.Rx_Size - 1])
				{
					received_dataframe_process(&buffer_temp, UART_Frame.Rx_Size);
				}
			}
			
		}
	}
	
}

uint8_t received_dataframe_process(void *dat, uint8_t len)
{
	const struct uart_head_t *head = dat;
	uint8_t ret;
	
	if(head->type == data_host_uart_tx)
	{
	}
	else if(head->type == imu_msg)
	{
		const struct imu_msg_t *imu = dat;
		
//		float pitch, roll, yaw;
		mouse_state.y = (int8_t)(((last_angle.pitch * 1.0 / (1<<16)) - (imu->pitch * 1.0 / (1<<16))) * 12);
		mouse_state.x = (int8_t)((last_angle.yaw * 1.0 / (1<<16) - imu->yaw * 1.0 / (1<<16)) * 12);
		
		usb_send_struct *send = usb_dataframe_alloc(sizeof(usb_send_struct));
		if(send)
		{
			send->reportid = MOUSE_REPORT_ID;
			send->size = sizeof(mouse_state);
			memcpy(send->data, &mouse_state, sizeof(mouse_state));
			usb_dataframe_send(send);
		}
		
		
		last_angle.pitch = imu->pitch;
		last_angle.roll = imu->roll;
		last_angle.yaw = imu->yaw;
		
//		memcpy(&last_angle, (long*)imu->pitch, sizeof(last_angle));
		
		memcpy(adc_value, imu->glove_adc, sizeof(adc_value));
//		update_glove_angle(adc_value);
	}
	
	return ret;
}

uint8_t update_glove_angle(uint16_t *adc)
{
	uint8_t ret;
	uint8_t temp[5];
	
	
	calculation_adc_to_angle(adc, temp);
	memcpy(glove_state.glove_angle, temp, sizeof(temp));
	
	uint8_t string[32];
	uint8_t len = sprintf((char *)string, "adc,%d,%d,%d,%d,%d\r\n", temp[0], temp[1], temp[2], temp[3], temp[4]);
	
	usb_send_struct *send = usb_dataframe_alloc(sizeof(usb_send_struct));
	if(send)
	{
		send->reportid = GLOVE_REPORT_ID;
		send->size = len;
		memcpy(send->data, string, len);
		usb_dataframe_send(send);
	}
	
	return ret;
}

uint32_t times;
uint8_t calculation_adc_to_angle(uint16_t *adc, uint8_t *angle)
{
//	for(uint8_t i = 0; i < 5; i ++)
//	{
//		angle[i] = 100 + 10 * i;
//	}
	angle[0] = 99;
	angle[1] = 100;
	angle[2] = 101;
	angle[3] = 100;
	angle[4] = times++;
	
	return 1;
}

uint8_t usb_dataframe_send(void *dataframe)
{
	osStatus sta = osMailPut(mailid_of_usb_tx, dataframe);
	
	return sta;
}

void *usb_dataframe_alloc(size_t size)
{
	void *ret = NULL;
	
	if(size <= 128)
	{
		ret = osMailAlloc(mailid_of_usb_tx, 0);
	}
	
	if(ret == NULL)
	{
		return NULL;
	}
	
	return ret;
}

uint8_t usb_dataframe_free(void *dataframe)
{
	osStatus sta = osMailFree(mailid_of_usb_tx, dataframe);
	
	return sta;
}

uint8_t sy_checkSum(uint8_t *content, uint8_t len)
{
   uint8_t result = 0;
   for (int i = 0; i < len; i++) {
      result +=content[i];

   }
   return  ~result;
}


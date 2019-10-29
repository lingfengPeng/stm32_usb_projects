#include "thread_usb_outevent_process.h"


osThreadId usb_outevent_process_handler;

uint8_t usb_outevent_buf[64];

void init_thread_usb_outevent_process(void)
{
	osThreadDef(usb_outevent_process, thread_usb_outevent_process, osPriorityNormal, 0, 128);
	
	usb_outevent_process_handler = osThreadCreate(osThread(usb_outevent_process), NULL);
}


void thread_usb_outevent_process(void const *argument)
{
	
	for (;;)
	{
		osSignalWait(SIG_OUTEVENT, osWaitForever);
		char *proof = strstr((char *)usb_outevent_buf, "proof");
		if(proof != NULL)
		{
			if(*(proof + 6) == '0')
			{
				calibration_cmd_global = 0;
			}
			else if(*(proof + 6) == '1')
			{
				calibration_cmd_global = 1;
			}
			else
			{
				calibration_cmd_global = 2;
			}
			break;
		}
		
		char *transmission = strstr((char *)usb_outevent_buf, "transmission");
		if(transmission != NULL)
		{
			if(*(transmission + 13) == '1')
			{
				transfer_state = Transmit_Running;
			}
			else
			{
				transfer_state = Transmit_Stop;
			}
			break;
		}
	}
	
}




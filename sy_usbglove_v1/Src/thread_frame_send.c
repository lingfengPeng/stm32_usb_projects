#include "thread_frame_send.h"


osThreadId frame_send_handler;


void init_thread_frame_send(void)
{
	osThreadDef(frame_send, thread_frame_send, osPriorityNormal, 0, 128);
	frame_send_handler = osThreadCreate(osThread(frame_send), NULL);
}

void thread_frame_send(void const * argument)
{
	for(;;)
	{
		USBD_CUSTOM_HID_HandleTypeDef     *hhid = (USBD_CUSTOM_HID_HandleTypeDef*)hUsbDeviceFS.pClassData;
		while(hhid->state == CUSTOM_HID_IDLE)
		{
			osEvent evt = osMailGet(mailid_of_usb_tx, 0);
			
			
			if(evt.status == osEventMail && evt.value.p != NULL)
			{
				usb_send_struct *send = evt.value.p;
				
				
				if(1)//(transfer_state == Transmit_Running)
				{
					if(send->reportid == MOUSE_REPORT_ID)
					{
						USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, 0, (uint8_t *)send->data, send->size);
					}
					else if(send->reportid == GLOVE_REPORT_ID)
					{
						USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, 1, (uint8_t *)send->data, send->size);
					}
				}
				
				osMailFree(mailid_of_usb_tx, send);
			}
			
		}
		
			
	}
}




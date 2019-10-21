#include "thread_mouse_frame.h"


osThreadId mouse_frame_handler;


void init_thread_mouse_frame(void)
{
	osThreadDef(mouse_frame, thread_mouse_frame, osPriorityNormal, 0, 128);
	mouse_frame_handler = osThreadCreate(osThread(mouse_frame), NULL);
}

void thread_mouse_frame(void const * argument)
{
	for(;;)
	{
		osEvent evt = osSignalWait(SIG_USB_FRAME_MOUSE, 0);
		if(evt.status == osEventSignal)
		{
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&mouse_state, sizeof(mouse_state));
		}
		
	}
}




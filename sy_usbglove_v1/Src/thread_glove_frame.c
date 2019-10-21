#include "thread_glove_frame.h"

osThreadId glove_frame_handler;

void init_thread_glove_frame(void)
{
	osThreadDef(glove_frame, thread_glove_frame, osPriorityNormal, 0, 128);
	glove_frame_handler = osThreadCreate(osThread(glove_frame), NULL);
}

void thread_glove_frame(void const * argument)
{
	for(;;)
	{
		osEvent evt = osSignalWait(SIG_USB_FRAME_GLOVE, 0);
		if(evt.status == osEventSignal)
		{
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&glove_state, glove_state.size+1);
		}
		
	}
}

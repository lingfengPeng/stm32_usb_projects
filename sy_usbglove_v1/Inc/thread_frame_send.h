#ifndef __THREAD_FRAME_SEND_H__
#define __THREAD_FRAME_SEND_H__

#include "cmsis_os.h"

#include "thread_usbframe_process.h"

void init_thread_frame_send(void);
void thread_frame_send(void const * argument);

extern osThreadId frame_send_handler;



#endif

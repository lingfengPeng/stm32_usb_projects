#ifndef __THREAD_MOUSE_FRAME_H__
#define __THREAD_MOUSE_FRAME_H__


#include "cmsis_os.h"

#include "thread_usbframe_process.h"

void init_thread_mouse_frame(void);
void thread_mouse_frame(void const * argument);

extern osThreadId mouse_frame_handler;


#endif

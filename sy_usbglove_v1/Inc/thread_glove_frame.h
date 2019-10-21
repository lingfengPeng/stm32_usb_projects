#ifndef __THREAD_GLOVE_FRAME_H__
#define __THREAD_GLOVE_FRAME_H__


#include "cmsis_os.h"

#include "thread_usbframe_process.h"


void init_thread_glove_frame(void);
void thread_glove_frame(void const * argument);

extern osThreadId glove_frame_handler;

#endif


#ifndef __THREAD_USB_OUTEVENT_PROCESS_H__
#define __THREAD_USB_OUTEVENT_PROCESS_H__


#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include "thread_usbframe_process.h"
#include <string.h>

#define SIG_OUTEVENT	0x02


void init_thread_usb_outevent_process(void);
void thread_usb_outevent_process(void const *argument);

extern uint8_t calibration_cmd_global;
extern uint8_t usb_outevent_buf[64];
extern osThreadId usb_outevent_process_handler;

#endif

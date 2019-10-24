#ifndef __THREAD_BLE_TRANSFER_H__
#define __THREAD_BLE_TRANSFER_H__

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"


#include "thread_usbframe_process.h"


#define MAX_SIZE_OF_SERIAL_DATAGRAM_EVENT 128

extern osThreadId ble_transfer_handler;
extern osMailQId mailid_of_ble_tx;

void init_thread_ble_transfer(void);
void thread_ble_transfer(void const * argument);
void thread_setup_data_frame_tx(void const *argument);

void *SerialDatagramEvtAlloc(size_t size);
int SerialDatagramEvtSend(void *ptr);


#endif

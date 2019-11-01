#ifndef __UART_API_H__
#define __UART_API_H__

#include "cmsis_os.h"
#include "misc.h"
 

void InitUartIo(void);


// 2nd paramter of callback of UART_IO function
enum StopUartReason_t {
	UartRxFinished    = 0,
	UartTxFinished    = 1,
	UartErrorOccurred = 2,
};

osStatus StartUartTx(uint8_t No, const uint8_t *pData, uint16_t Size, IoCallback_t Callback, void* CallbackArgument1);
osStatus StartUartRx(uint8_t No, uint8_t *pData, uint16_t Size, uint16_t Delimiter, IoCallback_t Callback, void* CallbackArgument1);
void StopUartTx(uint8_t No);
void StopUartRx(uint8_t No);



#endif

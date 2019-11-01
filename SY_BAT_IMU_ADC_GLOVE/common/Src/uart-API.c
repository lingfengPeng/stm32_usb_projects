#include "uart-API.h"

#include "stm32f1xx_hal.h"
#include <ctype.h>

#include "imu_me1_uart_task.h"
#include "ble_wt-bt200a_uart_task.h"
#pragma O3

#define DISABLE_IRQ() __disable_irq()
#define ENABLE_IRQ()  __enable_irq()
//define USART2_IRQHandler   _a_unused_USART2_IRQHandler_  // to make that function be forgotten
//#define UART5_IRQHandler    _a_unused_UART5_IRQHandler_   // to make that function be forgotten
//#define UART4_IRQHandler    _a_unused_UART4_IRQHandler_   // to make that function be forgotten

typedef struct
{
	USART_TypeDef*	Instance;			// UART registers
	IRQn_Type		IrqNo;
	const uint8_t*	TxBuffPtr;
	uint16_t		TxXferCount;
	uint8_t*		RxBuffPtr;
	uint16_t		RxXferCount;
	uint16_t		RxBufCount;
	IoCallback_t	RxCallback;
	void*			RxCallbackArgument1;
	IoCallback_t	TxCallback;
	void*			TxCallbackArgument1;
	uint16_t		RxEOL;				// EOL of receive data; if none, make it bigger then 255U
} UART_IO_t;

#ifndef	STM32F103xB
static UART_IO_t UartIoList[] = {
	{USART1, USART1_IRQn},
	{USART2, USART2_IRQn},
	{USART3, USART3_IRQn},
	{UART4 , UART4_IRQn },
	{UART5 , UART5_IRQn },
};
#else
static UART_IO_t UartIoList[] = {
	{USART1, USART1_IRQn},
	{USART2, USART2_IRQn},
	{USART3, USART3_IRQn},
};

#endif

#define UartIoOf(no) (UartIoList+(no) - 1)

void InitUartIo(void)
{
}

osStatus StartUartTx(uint8_t No, const uint8_t *pData, uint16_t Size, IoCallback_t Callback, void* CallbackArgument1)
{
	UART_IO_t *UartIo = UartIoOf(No);
	if (No < 1 || No > 5) {
		// no such device
		return osErrorValue;
	}
	
	UartIo->TxBuffPtr            = pData;
	UartIo->TxXferCount          = Size;
	UartIo->TxCallback           = Callback;
	UartIo->TxCallbackArgument1  = CallbackArgument1;
    
	DISABLE_IRQ();
	__HAL_UART_ENABLE_IT(UartIo, UART_IT_TXE);
	ENABLE_IRQ();
	return osOK;
}

void StopUartTx(uint8_t No)
{
	UART_IO_t *UartIo = UartIoOf(No);
	DISABLE_IRQ();
	__HAL_UART_DISABLE_IT(UartIo, UART_IT_TXE);
	__HAL_UART_DISABLE_IT(UartIo, UART_IT_TC);
	ENABLE_IRQ();
}

osStatus StartUartRx(uint8_t No, uint8_t *pData, uint16_t Size, uint16_t Delimiter, IoCallback_t Callback, void* CallbackArgument1)
{
	UART_IO_t *UartIo  = UartIoOf(No);
	if (No < 1 || No > 5) {
		// no such device
		return osErrorValue;
	}

	UartIo->RxBuffPtr            = pData;
	UartIo->RxXferCount          = Size;
	UartIo->RxBufCount           = Size;
	UartIo->RxEOL				 = Delimiter;
	UartIo->RxCallback           = Callback;
	UartIo->RxCallbackArgument1  = CallbackArgument1;
    
	DISABLE_IRQ();
	__HAL_UART_ENABLE_IT(UartIo, UART_IT_ERR);
	__HAL_UART_ENABLE_IT(UartIo, UART_IT_RXNE);
	ENABLE_IRQ();
	return osOK;
}

void StopUartRx(uint8_t No)
{
	UART_IO_t *UartIo = UartIoOf(No);
	DISABLE_IRQ();
	__HAL_UART_DISABLE_IT(UartIo, UART_IT_RXNE);
	__HAL_UART_DISABLE_IT(UartIo, UART_IT_ERR);
	ENABLE_IRQ();
}

// ISR context

static void UART_IRQHandler(UART_IO_t *UartIo);

void USART1_IRQHandler(void)
{
	UART_IO_t *UartIo = UartIoOf(1);
	UART_IRQHandler(UartIo);
}

extern UART_HandleTypeDef huart2;
void USART2_IRQHandler(void)
{
	UART2_RxIdleCallback(&huart2);
	UART_IO_t *UartIo = UartIoOf(2);
	UART_IRQHandler(UartIo);
}
extern UART_HandleTypeDef huart3;
void USART3_IRQHandler(void)
{
	UART3_RxIdleCallback(&huart3);
	UART_IO_t *UartIo = UartIoOf(3);
	UART_IRQHandler(UartIo);
}

#ifndef STM32F103xB
void UART4_IRQHandler(void)
{
	UART_IO_t *UartIo = UartIoOf(4);
	UART_IRQHandler(UartIo);
}
void UART5_IRQHandler(void)
{
	UART_IO_t *UartIo = UartIoOf(5);
	UART_IRQHandler(UartIo);
}
#endif

static void StopUartIo(UART_IO_t *UartIo, enum StopUartReason_t Reason)
{
	switch(Reason) {
		
	case UartRxFinished:
	case UartErrorOccurred:
		__HAL_UART_DISABLE_IT(UartIo, UART_IT_RXNE);										//此处有隐患，但是不屏蔽这个会导致串口多余缓存无法清除
	
//此处由于调用__HAL_UART_DISABLE_IT(UartIo, UART_IT_RXNE) 导致RXNE为0，使SR和DR不可读，串口缓存未读出，ORE错误标志一直存在。
		__HAL_UART_DISABLE_IT(UartIo, UART_IT_ERR);
		if (UartIo->RxCallback) {
			UartIo->RxCallback(UartIo->RxCallbackArgument1, Reason == UartRxFinished ? UartIo->RxBufCount - UartIo->RxXferCount : 0);
		}
		break;
	
	case UartTxFinished:
		__HAL_UART_DISABLE_IT(UartIo, UART_IT_TXE);
		__HAL_UART_DISABLE_IT(UartIo, UART_IT_TC);
		if (UartIo->TxCallback) {
			UartIo->TxCallback(UartIo->TxCallbackArgument1, Reason);
		}
		break;
	}
}

/**
  * @brief  This function handles UART interrupt request.
  * @param  UartIo: Pointer to a UART_IO_t structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
static void UART_IRQHandler(UART_IO_t *UartIo)
{
    uint32_t tmp_flag = 0, tmp_it_source = 0;
    if(__HAL_UART_GET_IT_SOURCE(UartIo, UART_IT_ERR) != RESET) {
		uint32_t HasError = 0;
		uint32_t flag_pe  = __HAL_UART_GET_FLAG(UartIo, UART_FLAG_PE);
		uint32_t flag_fe  = __HAL_UART_GET_FLAG(UartIo, UART_FLAG_FE);
		uint32_t flag_ne  = __HAL_UART_GET_FLAG(UartIo, UART_FLAG_NE);
		uint32_t flag_ore = __HAL_UART_GET_FLAG(UartIo, UART_FLAG_ORE);
		HasError = HasError || flag_pe;
		HasError = HasError || flag_fe;
		HasError = HasError || flag_ne;
		HasError = HasError || flag_ore;
		if (HasError) {
			// Error occured
			__HAL_UART_CLEAR_PEFLAG (UartIo);
			__HAL_UART_CLEAR_FEFLAG (UartIo);
			__HAL_UART_CLEAR_NEFLAG (UartIo);
			__HAL_UART_CLEAR_OREFLAG(UartIo);

			StopUartIo(UartIo, UartErrorOccurred); // Error Stop
			return;
		}
    }
    
    tmp_flag = __HAL_UART_GET_FLAG(UartIo, UART_FLAG_RXNE);
    tmp_it_source = __HAL_UART_GET_IT_SOURCE(UartIo, UART_IT_RXNE);
    /* UART in mode Receiver ---------------------------------------------------*/
    if((tmp_flag != RESET) && (tmp_it_source != RESET)) {
		// Receing...
		uint8_t byte = (uint8_t)(UartIo->Instance->DR & (uint8_t)0x00FF);
		*UartIo->RxBuffPtr++ = byte;
		--UartIo->RxXferCount;
		if ((UartIo->RxXferCount == 0) || byte == UartIo->RxEOL) {
			StopUartIo(UartIo, UartRxFinished); // no error stop at Receiving
			return;
		}
    }
    
    tmp_flag = __HAL_UART_GET_FLAG(UartIo, UART_FLAG_TXE);
    tmp_it_source = __HAL_UART_GET_IT_SOURCE(UartIo, UART_IT_TXE);
    /* UART in mode Transmitter ------------------------------------------------*/
    if((tmp_flag != RESET) && (tmp_it_source != RESET)) {
		if (UartIo->IrqNo == USART1_IRQn) { // data check, it isn't a driver's job, but.. it might be helpful.
			uint8_t c;
			c = UartIo->TxBuffPtr[0];
			if (!isxdigit(c) && !isspace(c)) {
//				global_run_info.total_send_datagram_sendout_error++; // a very huge error!
				StopUartIo(UartIo, UartTxFinished);
				return;
			}
		}
		UartIo->Instance->DR = (uint8_t)(*UartIo->TxBuffPtr++ & (uint8_t)0x00FF);
		if(--UartIo->TxXferCount == 0) {
			__HAL_UART_DISABLE_IT(UartIo, UART_IT_TXE);
			__HAL_UART_ENABLE_IT(UartIo, UART_IT_TC);
		}
    }

	tmp_flag = __HAL_UART_GET_FLAG(UartIo, UART_FLAG_TC);
	tmp_it_source = __HAL_UART_GET_IT_SOURCE(UartIo, UART_IT_TC);
	/* UART in mode Transmitter end --------------------------------------------*/
	if((tmp_flag != RESET) && (tmp_it_source != RESET)){
		StopUartIo(UartIo, UartTxFinished); // no error stop at Transmition
	}  
}

#ifndef __UART_LINE_IO_H__
#define __UART_LINE_IO_H__

#include <stdint.h>

// 2 simple warper of UART API

// return 0 if error;
// return > 0 means byte to read
uint16_t GetLineFromUart(uint8_t No, uint8_t *Line, uint16_t MaxLen, uint16_t Delimiter, uint32_t millisec);
uint16_t GetDataFromUart(uint8_t No, uint8_t *Line, uint16_t MaxLen, uint32_t millisec);

int SendDataToUart(uint8_t No, uint8_t *buf, uint16_t max_len, uint32_t millisec);

// send and receive together.
int SendReqAndRecvResDataWithUart(uint8_t No, uint8_t *req, uint16_t req_len, uint8_t *res, uint16_t res_len, uint32_t millisec);
int SendReqAndRecvResLineWithUart(uint8_t No, uint8_t *req, uint16_t req_len, uint8_t *res, uint16_t res_len, uint16_t Delimiter, uint32_t millisec);

#endif

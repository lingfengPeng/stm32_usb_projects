#include "uart-API.h"

// 2 simple warper of UART API


uint16_t GetLineFromUart(uint8_t No, uint8_t *Line, uint16_t MaxLen, uint16_t Delimiter, uint32_t millisec)
{
	if (No < 1 || No > 5) {
		// no such device
		return 0;
	}

	struct AsyncIoResult_t IoResult = { 0, osThreadGetId() };
	osStatus status = StartUartRx(No, Line, MaxLen, Delimiter, NotifyAsyncIoFinished, &IoResult);
	if (status != osOK) {
		return 0;
	}
	osEvent wait_event = osSignalWait(SIG_SERVER_FINISHED, millisec);
	if (wait_event.status == osEventTimeout) {
		// no signal sent to me, then Stop IO
		StopUartRx(No);
		return 0;
	}

	return IoResult.IoResult;
}

uint16_t GetDataFromUart(uint8_t No, uint8_t *Line, uint16_t MaxLen, uint32_t millisec)
{
	uint16_t ret = GetLineFromUart(No, Line, MaxLen, 0x100U, millisec); // 0x100 is not any ASCII, means 'get every thing'
	return ret;
}

int SendDataToUart(uint8_t No, uint8_t *buf, uint16_t max_len, uint32_t millisec)
{
	if (No < 1 || No > 5) {
		// no such device
		return 0;
	}

	struct AsyncIoResult_t IoResult = { 0, osThreadGetId() };
	osStatus status = StartUartTx(No, buf, max_len, NotifyAsyncIoFinished, &IoResult);
	osEvent wait_event = osSignalWait(SIG_SERVER_FINISHED, millisec);
	if (wait_event.status == osEventTimeout) {
		// no signal sent to me, then Stop IO
		StopUartTx(No);
		return 0;
	}
	return 1;
}

static void NotifyAsyncIoFinishedWithSignal(void *p, uint32_t value, int32_t signal)
{
	struct AsyncIoResult_t *result = p;
	result->IoResult = value;
	osSignalSet(result->CallerId, signal);
}

#define SIG_PHASE_REQ SIG_USER_0 
#define SIG_PHASE_RES SIG_USER_1
static void NotifyAsyncIoFinishedAtReqPhase(void *p, uint32_t value)
{
	NotifyAsyncIoFinishedWithSignal(p, value, SIG_PHASE_REQ);
}
static void NotifyAsyncIoFinishedAtResPhase(void *p, uint32_t value)
{
	NotifyAsyncIoFinishedWithSignal(p, value, SIG_PHASE_RES);
}

int SendReqAndRecvResLineWithUart(uint8_t No, uint8_t *req, uint16_t req_len, uint8_t *res, uint16_t res_len, uint16_t Delimiter, uint32_t millisec)
{
	struct AsyncIoResult_t ReqIoResult = { 0, osThreadGetId() };
	struct AsyncIoResult_t ResIoResult = { 0, osThreadGetId() };
	osStatus status;
	status = StartUartRx(No, res, res_len, Delimiter, NotifyAsyncIoFinishedAtResPhase, &ResIoResult);
	if (status != osOK) {
		return 0;
	}
	status = StartUartTx(No, req, req_len, NotifyAsyncIoFinishedAtReqPhase, &ReqIoResult);
	if (status != osOK) {
		StopUartRx(No);
		return 0;
	}
	osEvent wait_event = osSignalWait(SIG_PHASE_REQ|SIG_PHASE_RES, millisec);
	if (wait_event.status == osEventTimeout) {
		// no signal sent to me, then Stop IO
		StopUartTx(No);
		StopUartRx(No);
		return 0;
	}
	return ResIoResult.IoResult;
}

int SendReqAndRecvResDataWithUart(uint8_t No, uint8_t *req, uint16_t req_len, uint8_t *res, uint16_t res_len, uint32_t millisec)
{
	int ret = SendReqAndRecvResLineWithUart(No, req, req_len, res, res_len, 0x100U, millisec); // 0x100 is not any ASCII, means 'get every thing'
	return ret;
}


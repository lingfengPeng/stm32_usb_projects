#ifndef __MISC_H__
#define __MISC_H__

#include "cmsis_os.h"

// IPC definition
// notifiy Signal
#define SIG_SERVER_FINISHED           1
#define SIG_SERVER_ERROR              2
#define SIG_SERVER_TERMINATED         4
#define SIG_CLIENT_TERMINATE          8
#define SIG_USER_TIMER               16
#define SIG_USER_0                   32
#define SIG_USER_1                   64
#define SIG_USER_2                  128
#define SIG_USER_3                  256


typedef  void(*IoCallback_t)(void *, uint32_t);

struct AsyncIoResult_t {
	uint32_t IoResult;
	osThreadId CallerId;
};
void NotifyAsyncIoFinished(void *p, uint32_t value); // example of IoCallback_t. it will send signal SIG_SERVER_FINISHED to the thread.

// User timer
typedef struct {
	osThreadId tid;
	int32_t signal;
} user_signal_info_t;

typedef struct usrctl
{
	const char				*pcTimerName;		
	ListItem_t				xTimerListItem;		
	TickType_t				xTimerPeriodInTicks;
	UBaseType_t				uxAutoReload;		
	void 					*pvTimerID;			
} usrTIMER;

void SetUserSignal(const void *user_signal_info_ptr);

// MCU send text info to the Host by serial-datagram event.
// max size: 71 bytes.
// it's for debugging.
void SendStrToHost(const char *msg);
void PrintToHost(const char *format,...);

uint32_t          HAL_GetTick(void);

void set_thread_name(const char *name);
#endif

#include "misc.h"
void NotifyAsyncIoFinished(void *p, uint32_t value)
{
	struct AsyncIoResult_t *result = p;
	result->IoResult = value;
	osSignalSet(result->CallerId, SIG_SERVER_FINISHED);
}


void SetUserSignal(const void *user_signal_info_ptr)
{
	const usrTIMER *s = user_signal_info_ptr;
	const user_signal_info_t *p = s->pvTimerID;
	osSignalSet(p->tid, p->signal);
}


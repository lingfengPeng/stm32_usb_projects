#ifndef __BLE_WHBT200A_UART_TASK_H__
#define __BLE_WHBT200A_UART_TASK_H__

#include "stm32f1xx_hal.h"
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"

#include "misc.h"
#include "uart-API.h"

int init_thread_of_host_uart_tx (void);
int init_thread_of_host_uart_rx (void);
int init_ble_reset_task(void);

// datagram description
#define SERIAL_DATAGRAM_START_CHR '\r'
#define SERIAL_DATAGRAM_END_CHR   '\n'

#define FUNC_TAB_ITEM(type) {type, sizeof(struct serial_##type##_t) , &mail_queue_id_for_cmd_##type }

#define MAX_SIZE_OF_SERIAL_DATAGRAM_EVENT 128


enum evt_id_t{
	data_host_uart_tx = 5,
	calibration_resualt,
	calibration_data,
	software_version,
	machinary_id,
	serial_calibration_noti,	
	cmd_id_min,
	rawdata_tx_cmd,
	cmd_id_max = 0x0f,	
	imu_msg,	
	imu_debug,
	evt_id_max,
};

enum cmd_id_t{
	calibration_cmd = cmd_id_min,
};

struct uart_head_t{
	uint8_t serial_tail_h;
	uint8_t serial_tail_l;
	uint8_t type;
	uint8_t body_len;	
};

struct data_host_uart_tx_t{
	struct uart_head_t head;
	uint16_t adc[5];
};

struct rawdata_tx_cmd_t{
	struct uart_head_t head;
	uint16_t adc_value[5];
};

struct calibration_resualt_t{
	struct uart_head_t head;
	uint16_t tx_buff;
};

struct calibration_data_t{
	struct uart_head_t head;
	uint16_t tx_buff[10];
};

struct software_version_t{
	struct uart_head_t head;
	uint16_t tx_buff[3];
};

struct machinary_id_t{
	struct uart_head_t head;
	uint16_t tx_buff[2];
};

struct serial_calibration_cmd_t{
	struct uart_head_t head;	
	uint16_t cmd;
};

struct serial_calibration_noti_t{
	struct uart_head_t head;	
};

struct gloves_msg_process_func_t{
	uint8_t id;
	uint8_t body_len;
	const void *ptr;
};

struct imu_msg_t{
	struct uart_head_t head;
//	int16_t acc[3];
//	int16_t velo[3];
	int16_t angle[3];
	uint16_t adc[5];
};

struct imu_debug_t{
	struct uart_head_t head;
	uint8_t data[33];
};


#define DECL_MAIL_QUEUE_FOR_SERIAL_DATAGRAM_CMD(type) extern osMailQId mail_queue_id_for_cmd_##type;
DECL_MAIL_QUEUE_FOR_SERIAL_DATAGRAM_CMD(calibration_cmd)

// following should be used in our .c not invoker
#define DEFINE_MAIL_QUEUE_FOR_SERIAL_DATAGRAM_CMD(type,n) osMailQDef(type, (n), struct serial_##type##_t); osMailQId mail_queue_id_for_cmd_##type;
#define INIT_MAIL_QUEUE_FOR_SERIAL_DATAGRAM_CMD(type) mail_queue_id_for_cmd_##type = osMailCreate(osMailQ(type), NULL)



#define SERIAL_DATAGRAM_INIT(name, id)          \
	memset(&name, 0, sizeof name);                \
	name.head.serial_tail_h = 0xFE;								\
	name.head.serial_tail_l = 0xFE;								\
	name.head.type       	 	= id;                 \
	name.head.body_len	 	  = sizeof(struct id##_t);

extern osThreadId tid_thread_of_sensor_calibration;

// prepair stage: allocate a mail box slot for event. size is serial-datagram-size. return NULL if failed, non-NULL which pointer to the serial-datagram.
void *SerialDatagramEvtAlloc(size_t size);
// send data into the mail Q, then sending thread(event thread) would sending it 
int SerialDatagramEvtSend(void *ptr);
void SerialDatagramEvtFree(void *ptr);
int host_uart_datagram_send(void *msg, const size_t msg_len);
int send_raw_datagram_to_serial(const void *raw_datagram, size_t raw_datagram_len);

int get_raw_datagram_from_serial(uint8_t *raw_datagram, size_t max_size, size_t *actual_size_ptr, size_t *skipped_byte_count_ptr);
static int serial_datagram_msg_process_common_func(const void *msg, size_t msg_len, const void *ptr);
uint8_t sy_checkSum(uint8_t *content, uint8_t len);
void UART2_RxIdleCallback(UART_HandleTypeDef *huart);
void ble_rev_process(const void *msg, size_t msg_len);
#endif
/****************************************END OF FILE**************************************/

#ifndef	__THREAD_USBFRAME_PROCESS_H__
#define	__THREAD_USBFRAME_PROCESS_H__

#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include <string.h>

#include "usart.h"
#include "usbd_customhid.h"



#define SIG_USB_FRAME_MOUSE	0x0010
#define SIG_USB_FRAME_GLOVE	0x0020

#define MOUSE_REPORT_ID 1
#define GLOVE_REPORT_ID 2



void init_thread_usbframe_process(void);
void thread_usbframe_process(void const * argument);
uint8_t received_dataframe_process(void *dat, uint8_t len);

typedef enum
{
	Transmit_Running,
	Transmit_Stop,
}transmit_state_t;

//消息ID
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

struct uart_head_t{
	uint8_t serial_tail_h;
	uint8_t serial_tail_l;
	uint8_t type;
	uint8_t body_len;
};

struct data_host_uart_rx_t{
	struct uart_head_t head;
	uint16_t glove_adc[5];
};

//IMU:
struct imu_msg_t{
	struct uart_head_t head;
//	int16_t acc[3];
//	int16_t velo[3];
	int16_t angle[3];
	uint16_t glove_adc[5];
};

typedef struct angle_imu_tag{
	int16_t	pitch;
	int16_t roll;
	int16_t yaw;
}imu_angle_t;

typedef struct usb_head_tag{
	uint8_t reportid;
	uint8_t size;
}usb_head;

typedef struct usb_send_tag{
//	usb_head head;
	uint8_t reportid;
	uint8_t data[32];
	uint8_t size;
}usb_send_struct;

typedef struct usb_hid_tag{
	int8_t button;
	int8_t x;
	int8_t y;
	int8_t wheel;
}usb_hid_state_t;

typedef struct usb_adc_tag{
	uint8_t glove_angle[5];
}usb_adc_state_t;


//数据帧头
#define SERIAL_DATAGRAM_INIT(name, id)          \
	memset(&name, 0, sizeof name);                \
	name.head.serial_tail_h = 0xFE;								\
	name.head.serial_tail_l = 0xFE;								\
	name.head.type       	 	= id;                 \
	name.head.body_len	 	  = sizeof(struct id##_t);


extern USBD_HandleTypeDef hUsbDeviceFS;
extern osThreadId usbframe_process_handler;
extern imu_angle_t last_angle;
extern usb_hid_state_t mouse_state;
extern usb_adc_state_t glove_state;
extern osMailQId mailid_of_usb_tx;
extern transmit_state_t transfer_state;


uint8_t update_glove_angle(uint16_t *adc);
uint8_t calculation_adc_to_angle(uint16_t *adc, uint8_t *angle);

void *usb_dataframe_alloc(size_t size);
uint8_t usb_dataframe_send(void *dataframe);
uint8_t usb_dataframe_free(void *dataframe);



//校验函数(与HR08相同)
uint8_t sy_checkSum(uint8_t *content, uint8_t len);


#endif

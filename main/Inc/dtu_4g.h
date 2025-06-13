#ifndef __DTU_4G_H__
#define __DTU_4G_H__


#include "project_system.h"
#include "driver/uart.h"

#define DTU_4G_FIFO_NUM   20

typedef struct _dtu_4g_fifo
{
    /* data */
    uint8_t data[DTU_4G_FIFO_NUM][1024];
    uint16_t len[DTU_4G_FIFO_NUM];
    uint16_t pos;
    uint16_t tail;
}dtu_4g_fifo_t;

typedef enum _dtu_4g_comm_status
{
    DTU_4G_COMM_CONFIG = 0,
    DTU_4G_COMM_WORK = 1,
}dtu_4g_comm_status_t;

typedef enum _dtu_4g_process
{
    DTU_4G_PROCESS_REGIS_TOPICAL_CFG = 0,
    DTU_4G_PROCESS_REGISTER          = 1,
    DTU_4G_PROCESS_MSG_TOPICAL_CFG   = 2,
    DTU_4G_PROCESS_MSG               = 3,
}dtu_4g_process_t;

typedef enum _dtu_4g_use_status
{
    DTU_4G_NOT_USE = 0,
    DTU_4G_USING   = 1,
}dtu_4g_use_status_t;

typedef struct _dtu_4g_device
{
    /* data */
    uart_port_t uart_port;
    QueueHandle_t uart_queue;

    dtu_4g_fifo_t rx_fifo;
    dtu_4g_fifo_t tx_fifo;

    dtu_4g_process_t     process;
    dtu_4g_use_status_t  use_status;
    dtu_4g_comm_status_t comm_status;
    uint8_t comm_ack;
    int uart_baudrate;
    
}dtu_4g_device_t;

extern dtu_4g_device_t dtu_4g_device;

void dtu_4g_device_init(dtu_4g_device_t *dtu_4g_dev);


#endif



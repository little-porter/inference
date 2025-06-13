#include "rs485.h"

static const char *TAG = "PRJ_RS485";

#define RS485_UART_PORT     UART_NUM_2
#define RS485_UART_BAUD     9600
#define RS485_UART_TX_PIN   GPIO_NUM_17
#define RS485_UART_RX_PIN   GPIO_NUM_18
#define RS485_EN_PIN        GPIO_NUM_21


#define rs485_tx_enable()   gpio_set_level(RS485_EN_PIN,1)
#define rs485_tx_disable()  gpio_set_level(RS485_EN_PIN,0)

rs485_driver_t rs485_driver;


void rs485_send_task_handler(void *pvParameters);
void rs485_recive_task_handler(void *pvParameters);

void rs485_uart_init(rs485_driver_t *rs485_drv)
{
    uart_config_t uart_cfg = {
        .baud_rate = RS485_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,       
    };
    uart_param_config(RS485_UART_PORT, &uart_cfg);

    uart_set_pin(RS485_UART_PORT, RS485_UART_TX_PIN, RS485_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    rs485_drv->uart_queue = NULL;
    uart_driver_install(RS485_UART_PORT, 1024, 1024, 10, &rs485_drv->uart_queue, 0);

    if(rs485_drv->uart_queue == NULL)
    {
        ESP_LOGE(TAG, "uart driver install failed");
    }
}


void rs485_driver_init(rs485_driver_t *rs485_drv)
{
    memset(rs485_drv,0,sizeof(rs485_driver_t));
    rs485_drv->uart_port =  RS485_UART_PORT;
    rs485_uart_init(rs485_drv);

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // 禁用中断
    io_conf.mode = GPIO_MODE_OUTPUT; // 设置为输出模式
    io_conf.pin_bit_mask = (1ULL << RS485_EN_PIN); // 选择具体的GPIO
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // 禁用下拉电阻
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE; // 禁用上拉电阻
    gpio_config(&io_conf);

    xTaskCreatePinnedToCore(rs485_send_task_handler,"rs485_send_task",1024*2,rs485_drv,5,NULL,0);
    xTaskCreatePinnedToCore(rs485_recive_task_handler,"rs485_recive_task",1024*5,rs485_drv,5,NULL,0);

    ESP_LOGI(TAG,"RS485 init success!");
}

void rs485_push_data_to_tx_fifo(rs485_driver_t *rs485_drv,uint8_t *data,uint16_t len)
{
    memcpy(rs485_drv->tx_fifo.data[rs485_drv->tx_fifo.tail],data,len);
    rs485_drv->tx_fifo.len[rs485_drv->tx_fifo.tail] = len;
    rs485_drv->tx_fifo.tail++;
    rs485_drv->tx_fifo.tail %= RS485_FIFO_NUM;
}


void rs485_send_task_handler(void *pvParameters)
{
    rs485_driver_t *rs485_drv = (rs485_driver_t *)pvParameters;
    while (1)
    {
        while(rs485_drv->tx_fifo.pos != rs485_drv->tx_fifo.tail)
        {
            rs485_tx_enable();
            uart_write_bytes(rs485_drv->uart_port,rs485_drv->tx_fifo.data[rs485_drv->tx_fifo.pos],rs485_drv->tx_fifo.len[rs485_drv->tx_fifo.pos]);
            uart_wait_tx_done(rs485_drv->uart_port,portMAX_DELAY);
            rs485_drv->tx_fifo.pos++;
            rs485_drv->tx_fifo.pos %= RS485_FIFO_NUM;
            rs485_tx_disable();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
}

void rs485_recive_task_handler(void *pvParameters)
{
    rs485_driver_t *rs485_drv = (rs485_driver_t *)pvParameters;
    uint8_t rx_data[200];
#ifdef USE_BMS_DATA
    bms_msg_t bms_msg;
#endif
    while(1)
    {
        uart_event_t event;
        int rx_bytes = 0;
        if(pdTRUE == xQueueReceive(rs485_drv->uart_queue,&event,portMAX_DELAY))
        {
            switch (event.type)
            {
                case UART_DATA:
                    rx_bytes = uart_read_bytes(rs485_drv->uart_port, rx_data, event.size, 100 / portTICK_PERIOD_MS);
                    rx_data[rx_bytes] = 0x00;
                    ESP_LOGI(TAG,"*********************************************");
                    ESP_LOGI(TAG,"rs485 接收数量：%d\r\n",rx_bytes);
#ifdef USE_BMS_DATA
                    memset(&bms_msg,0,sizeof(bms_msg_t));
                    memcpy(bms_msg.data,rx_data,rx_bytes);
                    bms_msg.num = rx_bytes;
                    if(bms_rx_queue != NULL)
                        xQueueSend(bms_rx_queue,&bms_msg,portMAX_DELAY);
#endif
#ifdef USE_NOT_DEVCE
                    memset(rs485_drv->rx_fifo.data[rs485_drv->rx_fifo.tail],0,sizeof(rs485_fifo_t));
                    memcpy(rs485_drv->rx_fifo.data[rs485_drv->rx_fifo.tail],rx_data,rx_bytes);
                    rs485_drv->rx_fifo.len[rs485_drv->rx_fifo.tail] = rx_bytes;
                    rs485_drv->rx_fifo.tail++;
                    rs485_drv->rx_fifo.tail %= RS485_FIFO_NUM;
#endif
                break;
                default:
                break;
            }  
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}




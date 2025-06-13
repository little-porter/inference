#include "dtu_4g.h"



static const char *TAG = "PRJ_4G";

#define URL  2.tcp.cpolar.top

#define DTU_4G_UART_PORT    UART_NUM_1
#define DTU_4G_UART_BAUD    460800
#define DTU_4G_UART_RX_PIN  GPIO_NUM_4
#define DTU_4G_UART_TX_PIN  GPIO_NUM_5


dtu_4g_device_t dtu_4g_device;

#define DTU_4G_BAUD_NUM  8
const int dtu_4g_baudrate_list[DTU_4G_BAUD_NUM]  = {9600,19200,38400,57600,115200,230400,460800,921600};




void dtu_4g_recive_task_handler(void *pvParameters);
void dtu_4g_send_task_handler(void *pvParameters);
void dtu_4g_uart_init(dtu_4g_device_t *dtu_4g_dev)
{
    uart_config_t uart_config = {
        .baud_rate = DTU_4G_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    //设置串口参数
    uart_param_config(DTU_4G_UART_PORT, &uart_config);

    uart_set_pin(DTU_4G_UART_PORT, DTU_4G_UART_TX_PIN, DTU_4G_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(DTU_4G_UART_PORT, 1024*4, 1024*4, 10, &dtu_4g_dev->uart_queue, 0);

}

void dtu_4g_config_ack_deal(dtu_4g_device_t *dtu_4g_dev,uint8_t  *data, int len)
{
    ESP_LOGI(TAG,"dtu_4g config ack deal!");
    if(0 == strncmp((char *)data,"\r\nconfig,uart,ok\r\n",len))
    {
        dtu_4g_dev->comm_ack = 1;
        ESP_LOGI(TAG,"config baud ok***************************************************");
    }
    else if(strncmp((char *)data,"\r\nconfig,uart,ok",len) > 0)
    {
        dtu_4g_dev->comm_ack = 1;
        ESP_LOGI(TAG,"read baud ok***************************************************");
    }
    else if(0 == strncmp((char *)data,"\r\nconfig,mqtt,ok\r\n",len))
    {
        dtu_4g_dev->comm_ack = 1;
        ESP_LOGI(TAG,"mqtt cofig ok***************************************************");
    }
}


void dtu_4g_msg_deal(dtu_4g_device_t *dtu_4g_dev,uint8_t  *data, int len)
{
    ESP_LOGI(TAG,"dtu_4g msg deal!");

    
}

void dtu_4g_recive_msg_deal(dtu_4g_device_t *dtu_4g_dev,uint8_t  *data, int len)
{
    switch (dtu_4g_dev->comm_status)
    {
    case DTU_4G_COMM_CONFIG:
    /* code */
        dtu_4g_config_ack_deal(dtu_4g_dev,data,len);
        break;
    case DTU_4G_COMM_WORK:
        dtu_4g_msg_deal(dtu_4g_dev,data,len);
        break;
    default:
        break;
    }
}


void dtu_4g_recive_task_handler(void *pvParameters)
{ 
    dtu_4g_device_t *dtu_4g_dev = (dtu_4g_device_t *)pvParameters;
    uint8_t rx_data[1024];
    while (1)
    {
        /* code */
        uart_event_t event;
        int rx_bytes = 0;
        if(pdTRUE == xQueueReceive(dtu_4g_dev->uart_queue,&event,portMAX_DELAY))
        {
            switch (event.type)
            {
            case UART_DATA:
                /* code */
                rx_bytes = uart_read_bytes(DTU_4G_UART_PORT, rx_data, event.size, portMAX_DELAY);
                rx_data[rx_bytes] = '\0';
                printf("uart rx data: %s\n",rx_data);
                dtu_4g_recive_msg_deal(dtu_4g_dev,rx_data,rx_bytes);

                break;
            
            default:
                break;
            }
        }

    }
    
}


void dtu_4g_get_uart_baudrate(dtu_4g_device_t *dtu_4g_dev);
void dtu_4g_set_uart_baudrate(dtu_4g_device_t *dtu_4g_dev,int baudrate);

void dtu_4g_send_task_handler(void *pvParameters)
{
    dtu_4g_device_t *dtu_4g_dev = (dtu_4g_device_t *)pvParameters;
    while (1)
    {
        // dtu_4g_get_uart_baudrate(dtu_4g_dev);
        // dtu_4g_set_uart_baudrate(dtu_4g_dev,9600);
        /* code */
        while (dtu_4g_dev->tx_fifo.pos != dtu_4g_dev->tx_fifo.tail)
        {
            /* code */
            uart_write_bytes(DTU_4G_UART_PORT,dtu_4g_dev->tx_fifo.data[dtu_4g_dev->tx_fifo.pos],dtu_4g_dev->tx_fifo.len[dtu_4g_dev->tx_fifo.pos]);
            uart_wait_tx_done(DTU_4G_UART_PORT,portMAX_DELAY);
            ESP_LOGI(TAG,"发送数据成功，tx_num = %d",dtu_4g_dev->tx_fifo.len[dtu_4g_dev->tx_fifo.pos]);
            dtu_4g_dev->tx_fifo.pos++;
            dtu_4g_dev->tx_fifo.pos %= DTU_4G_FIFO_NUM;
            

            vTaskDelay(pdMS_TO_TICKS(50));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
}


void dtu_4g_push_data_to_tx_fifo(dtu_4g_device_t *dtu_4g_dev,uint8_t *data,uint16_t len)
{
    memcpy(dtu_4g_dev->tx_fifo.data[dtu_4g_dev->tx_fifo.tail],data,len);
    dtu_4g_dev->tx_fifo.len[dtu_4g_dev->tx_fifo.tail] = len;
    dtu_4g_dev->tx_fifo.tail++;
    dtu_4g_dev->tx_fifo.tail %= DTU_4G_FIFO_NUM;
}

void dtu_4g_get_uart_baudrate(dtu_4g_device_t *dtu_4g_dev)
{
    char buffer[200];
    int lenth = snprintf(buffer, sizeof(buffer), "config,get,uart\r\n");
    dtu_4g_push_data_to_tx_fifo(dtu_4g_dev,(uint8_t *)buffer,lenth);
}

void dtu_4g_set_uart_baudrate(dtu_4g_device_t *dtu_4g_dev,int baudrate)
{
    char buffer[200];
    int lenth = snprintf(buffer, sizeof(buffer), "config,set,uart,%d,8,0,1,80\r\n",baudrate);
    dtu_4g_push_data_to_tx_fifo(dtu_4g_dev,(uint8_t *)buffer,lenth);
}

void dtu_4g_save_config(dtu_4g_device_t *dtu_4g_dev)
{
    char buffer[200];
    int lenth = snprintf(buffer, sizeof(buffer), "config,set,save\r\n");
    dtu_4g_push_data_to_tx_fifo(dtu_4g_dev,(uint8_t *)buffer,lenth);
}

void dtu_4g_reboot(dtu_4g_device_t *dtu_4g_dev)
{
    char buffer[200];
    int lenth = snprintf(buffer, sizeof(buffer), "config,set,reboot\r\n");
    dtu_4g_push_data_to_tx_fifo(dtu_4g_dev,(uint8_t *)buffer,lenth);
}

void dtu_4g_reset(dtu_4g_device_t *dtu_4g_dev)
{
    char buffer[200];
    int lenth = snprintf(buffer, sizeof(buffer), "config,set,reset\r\n");
    dtu_4g_push_data_to_tx_fifo(dtu_4g_dev,(uint8_t *)buffer,lenth);
}

void dtu_4g_baudrate_find(dtu_4g_device_t *dtu_4g_dev)
{
    int baudrate = 0;
    dtu_4g_dev->comm_ack = 0;
    /*默认配置*/
    dtu_4g_get_uart_baudrate(dtu_4g_dev);
    vTaskDelay(pdMS_TO_TICKS(5000));
    if(dtu_4g_dev->comm_ack)
    {
        dtu_4g_dev->uart_baudrate = baudrate;
        return;
    }
    /*遍历波特率列表匹配波特率*/
    for (int i = 0; i < DTU_4G_BAUD_NUM; i++)
    {
        baudrate = dtu_4g_baudrate_list[i];
        uart_set_baudrate(DTU_4G_UART_PORT,baudrate);
        dtu_4g_get_uart_baudrate(dtu_4g_dev);
        vTaskDelay(pdMS_TO_TICKS(5000));
        if(dtu_4g_dev->comm_ack)
        {
            dtu_4g_dev->uart_baudrate = baudrate;
            break;
        }
        /* code */
    }
    
}

void dtu_4g_uart_config(dtu_4g_device_t *dtu_4g_dev)
{
    dtu_4g_dev->comm_status = DTU_4G_COMM_CONFIG;
    dtu_4g_dev->comm_ack = 0;
    dtu_4g_baudrate_find(dtu_4g_dev);
    vTaskDelay(pdMS_TO_TICKS(5000));
    while (!dtu_4g_dev->comm_ack)
    {
        dtu_4g_set_uart_baudrate(dtu_4g_dev,DTU_4G_UART_BAUD);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    dtu_4g_save_config(dtu_4g_dev);
    vTaskDelay(pdMS_TO_TICKS(5000));
    uart_set_baudrate(DTU_4G_UART_PORT,DTU_4G_UART_BAUD);
    // dtu_4g_reboot(dtu_4g_dev);
    // vTaskDelay(pdMS_TO_TICKS(5000));
    // while(1)
    // {
    //     dtu_4g_get_uart_baudrate(dtu_4g_dev);
    //     vTaskDelay(pdMS_TO_TICKS(6000));
    // }
}

void dtu_4g_work_process_task_handler(void *pvParameters);
void dtu_4g_device_init(dtu_4g_device_t *dtu_4g_dev)
{ 
    dtu_4g_uart_init(dtu_4g_dev);
    dtu_4g_dev->comm_ack = 0;
    dtu_4g_dev->comm_status = DTU_4G_COMM_CONFIG;
    dtu_4g_dev->use_status = DTU_4G_USING;
    xTaskCreatePinnedToCore(dtu_4g_recive_task_handler,"4g_recive_task",1024*8,dtu_4g_dev,5,NULL,0);
    xTaskCreatePinnedToCore(dtu_4g_send_task_handler,"4g_send_task",1024*8,dtu_4g_dev,5,NULL,0);

    dtu_4g_uart_config(dtu_4g_dev);
    xTaskCreatePinnedToCore(dtu_4g_work_process_task_handler,"4g_work_task",1024*8,dtu_4g_dev,5,NULL,0);
}

void dtu_4g_register_topical_cfg(dtu_4g_device_t *dtu_4g_dev,int id)
{
    char buffer[200];
    char url[] = "2.tcp.cpolar.top";
    int  port = 11348;
    /*config,set,mqtt,  
                        通道，串口类型，心跳包间隔，服务器地址，服务器端口，                          (5)
                        用户id，用户名，密码，                                                     (3)
                        协议版本(0:3.1,1:3.1.1)，是否清除会话，持久消息，                           (3)
                        订阅QOS，发布QOS，订阅主题，发布主题，                                      (4) 
                        设置遗嘱，遗嘱QOS，遗嘱持久消息，遗嘱主题，遗嘱内容，注册数据类型，注册数据，   (7)
                        IPV4/6(0:IPV4,1:IPV6)，支持SSL就加密(0:不支持，1：支持无证书，2：支持有证书)  (2)
                        */                                       
    int lenth = snprintf(buffer, sizeof(buffer),"config,set,mqtt,"
                                                "1,uart,60,%s,%d,"
                                                "lithium_%d,admin,admin,"
                                                "1,1,0,"
                                                "0,0,lithium_battery/command/%d,lithium_battery/register,"
                                                "0,0,0,,,0,,"
                                                "0,0\r\n",
                                                url,port,id,id);
    printf(buffer);
    dtu_4g_push_data_to_tx_fifo(dtu_4g_dev,(uint8_t *)buffer,lenth);
}

void dtu_4g_tldevcie_register(dtu_4g_device_t *dtu_4g_dev)
{
    char buffer[200];
    // int lenth = snprintf()
}

void dtu_4g_msg_topical_cfg(dtu_4g_device_t *dtu_4g_dev,int id)
{
    char buffer[200];
    char url[] = "2.tcp.cpolar.top";
    int  port = 11348;
    /*config,set,mqtt,  
                        通道，串口类型，心跳包间隔，服务器地址，服务器端口，                          (5)
                        用户id，用户名，密码，                                                     (3)
                        协议版本(0:3.1,1:3.1.1)，是否清除会话，持久消息，                           (3)
                        订阅QOS，发布QOS，订阅主题，发布主题，                                      (4) 
                        设置遗嘱，遗嘱QOS，遗嘱持久消息，遗嘱主题，遗嘱内容，注册数据类型，注册数据，   (7)
                        IPV4/6(0:IPV4,1:IPV6)，支持SSL就加密(0:不支持，1：支持无证书，2：支持有证书)  (2)
                        */                                       
    int lenth = snprintf(buffer, sizeof(buffer),"config,set,mqtt,"
                                                "1,uart,60,%s,%d,"
                                                "lithium_%d,admin,admin,"
                                                "1,1,0,"
                                                "0,0,lithium_battery/command/%d,lithium_battery/upload,"
                                                "0,0,0,,,0,,"
                                                "0,0\r\n",
                                                url,port,id,id);
    printf(buffer);
    dtu_4g_push_data_to_tx_fifo(dtu_4g_dev,(uint8_t *)buffer,lenth);
}

void dtu_4g_work_process_task_handler(void *pvParameters)
{
    dtu_4g_device_t *dtu_4g_dev = (dtu_4g_device_t *)pvParameters;
    while(1)
    {
        if(dtu_4g_dev->use_status == DTU_4G_NOT_USE)
        {
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        switch (dtu_4g_dev->process)
        {
        case DTU_4G_PROCESS_REGIS_TOPICAL_CFG:
            /* code */
            dtu_4g_dev->comm_ack = 0;
            dtu_4g_dev->comm_status = DTU_4G_COMM_CONFIG;
            dtu_4g_register_topical_cfg(dtu_4g_dev,1);
            vTaskDelay(pdMS_TO_TICKS(5000));
            if(dtu_4g_dev->comm_ack == 1)
            {
                dtu_4g_save_config(dtu_4g_dev);
                dtu_4g_dev->comm_ack = 0;
                dtu_4g_dev->comm_status = DTU_4G_COMM_WORK;
                dtu_4g_dev->process = DTU_4G_PROCESS_REGISTER; 
                vTaskDelay(pdMS_TO_TICKS(5000)); 
            }
            break;
        case DTU_4G_PROCESS_REGISTER:
            dtu_4g_dev->comm_ack = 0;
            dtu_4g_dev->comm_status = DTU_4G_COMM_WORK;
            dtu_4g_tldevcie_register(dtu_4g_dev);
            vTaskDelay(pdMS_TO_TICKS(5000)); 
            if(dtu_4g_dev->comm_ack == 1)
            {
                dtu_4g_dev->comm_ack = 0;
                dtu_4g_dev->comm_status = DTU_4G_COMM_CONFIG;
                dtu_4g_dev->process = DTU_4G_PROCESS_MSG_TOPICAL_CFG;
            }
            break;
        case DTU_4G_PROCESS_MSG_TOPICAL_CFG:
            dtu_4g_dev->comm_ack = 0;
            dtu_4g_dev->comm_status = DTU_4G_COMM_CONFIG;
            dtu_4g_msg_topical_cfg(dtu_4g_dev,1);
            vTaskDelay(pdMS_TO_TICKS(5000));
            if(dtu_4g_dev->comm_ack == 1)
            {
                dtu_4g_save_config(dtu_4g_dev);
                dtu_4g_dev->comm_ack = 0;
                dtu_4g_dev->comm_status = DTU_4G_COMM_WORK;
                dtu_4g_dev->process = DTU_4G_PROCESS_MSG; 
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
            break;
        case DTU_4G_PROCESS_MSG:
            vTaskDelay(pdMS_TO_TICKS(5000));
            break;
        default:
            vTaskDelay(pdMS_TO_TICKS(5000)); 
            break;
        }
    }
}

#include "bms.h"
#include "rs485.h"
#include "inference.h"

static const char *TAG = "PRJ_BMS";

#define BMS_ID_24BYTES      0
#define BMS_ID_28BYTES      1

#define BMS_ID_TYPE         BMS_ID_24BYTES
#define BMS_CMD_READ_REG    0x03

#define BMS_ID_LEN_24       24
#define BMS_ID_LEN_28       28

#define BMS_PACK_CAPACITY   2

#define BMS_OFFLINE_TIME    10

//接收数据队列
QueueHandle_t  bms_rx_queue = NULL;
//任务句柄
TaskHandle_t bms_msg_task_handle = NULL;
TaskHandle_t bms_cmd_task_handle = NULL;
/*BMS设备信息*/
bms_device_t bms_device;

/*  BMS数据处理任务 */
void bms_device_msg_deal_task_handler(void *pvParameters);
/*  BMS数据查询任务 */
void bms_device_cmd_send_task_handler(void *pvParameters);
/* BMS crc计算*/
uint16_t bms_crc16_calculate(const void *s, int n)
{
    uint16_t c = 0xffff;
    for(int k=0; k<n; k++)
    {
        uint16_t b = (((uint8_t *)s)[k]);
        for(char i=0; i<8; i++)
        {
            c = ((b^c)&1) ? (c>>1)^0xA001 : (c>>1);
            b>>=1;
        }
    }
    return (c<<8)|(c>>8);
}
/* BMS 大小端数据交换（传输大端，存储小端） */
void bms_data_exchange(uint16_t *data,uint16_t len)
{
    uint8_t *p;
    uint16_t exData;
    
    for (uint16_t i = 0; i < len; i++)
    {
        p = (uint8_t *)(data+i);
        exData = *p<<8 | *(p+1);
        *(data+i) = exData;
    }
}

/* BMS初始化 */
void bms_device_init(bms_device_t *bms_dev)
{
    if(bms_dev == NULL)
    {
        ESP_LOGI(TAG, "bms_device_init: bms_dev is NULL");
        return;
    }

    bms_rx_queue = xQueueCreate(10, sizeof(bms_msg_t));
    bms_dev->status = BMS_OFFLINE;
    bms_dev->offline_time = BMS_OFFLINE_TIME;

    xTaskCreatePinnedToCore(bms_device_msg_deal_task_handler,"bms_msg_deal_task",1025*5,bms_dev,6,&bms_msg_task_handle,0);
    xTaskCreatePinnedToCore(bms_device_cmd_send_task_handler,"bms_cmd_send_task",1025*2,bms_dev,6,&bms_cmd_task_handle,0);

    ESP_LOGI(TAG, "bms_device init success!");
}



void bms_device_id_read(bms_device_t *bms_dev)
{
#if(BMS_ID_TYPE == BMS_ID_24BYTES)
    uint8_t bms_cmd[8]= {0x01, 0x03, 0x03, 0xE8, 0x00, 0x0C, 0xC5, 0xBF};
#else if(BMS_ID_TYPE == BMS_ID_28BYTES)
    uint8_t bms_cmd[8]= {0x01, 0x03, 0x03, 0xE8, 0x00, 0x0E, 0x44, 0x7E};
#endif
    /*通过rs485进行bms命令发送*/
    rs485_push_data_to_tx_fifo(&rs485_driver,bms_cmd,8);
}

void bms_device_reg_data_read(bms_device_t *bms_dev)
{
    uint8_t cmd[8] = {0x01,0x03,0x00,0x00,0x00,0x1F,0x00,0x00};

    uint16_t crc = bms_crc16_calculate(cmd,6);
    cmd[6] = crc>>8;
    cmd[7] = crc&0xff;
     /*通过rs485进行bms命令发送*/
    rs485_push_data_to_tx_fifo(&rs485_driver,cmd,8);
}


void bms_device_result_send(bms_device_t *bms_dev)
{
    extern inference_model_data_t *g_inference_data[MODEL_MAX_NUM];
    u8_f_data_t result;
    result.f_data = g_inference_data[0]->model_data.dx_data[0].inference_result[0];
    printf("bms send result:%f\n",result.f_data);
    uint8_t cmd[20] = {0};
    cmd[0] = 0x01;
    cmd[1] = 0xff;
    cmd[2] = 0x01;
    cmd[3] = 0x04;
    cmd[4] = result.u8_data[1];
    cmd[5] = result.u8_data[0];
    cmd[6] = result.u8_data[3];
    cmd[7] = result.u8_data[2];
    uint16_t crc = bms_crc16_calculate(cmd,8);
    cmd[8] = crc>>8;
    cmd[9] = crc&0xff;

    rs485_push_data_to_tx_fifo(&rs485_driver,cmd,10);
}

/*充放电状态获取*/
void bms_get_cfd_state(bms_device_t *bms_dev)
{
    float temp_state = 0;

    if(bms_dev->pack_fCurrent  > 0.02)
    {
        temp_state = 1.0f;          /*充电*/
    }
    else if(bms_dev->pack_fCurrent  < -0.02)
    {
        temp_state = 2.0f;          /*放电*/
    }
    else
    {
        temp_state = 0.0f;          /*待机*/
    }

    bms_dev->pack_cfd_zt = temp_state;
}

/*充放电时间计算*/
void  bms_get_cfd_time(bms_device_t *bms_dev)
{
    static uint8_t pre_CFDFlag = 1;             //记录上一次充放电状态

    if(!bms_dev->pack_cfd_zt) return;           /*待机状态不计算时间*/

    if(bms_dev->pack_cfd_zt != pre_CFDFlag)
    {
        bms_dev->pack_cfd_time = 0;             /*充放电状态改变，时间清零*/
        pre_CFDFlag = bms_dev->pack_cfd_zt;
    }
    else
    {
        bms_dev->pack_cfd_time++;               /*充放电时间自增*/
    }
    ESP_LOGI(TAG,"充放电状态：%d,时间:%d S",(int)bms_dev->pack_cfd_zt,(int)bms_dev->pack_cfd_time);
}

/*充放电率计算*/
void bms_get_cfd_C_rate(bms_device_t *bms_dev)
{
    bms_dev->pack_cfd_Crate = abs(bms_dev->pack_fCurrent)/ BMS_PACK_CAPACITY;        //充放电率 = 当前电流/ 容量（Crate = I/C）
}

/*充放电轮次计算 */
void bms_get_cfd_cycle(bms_device_t *bms_dev)
{
    static float fdl = 0;                            /*放电量记录*/
    // float fdl_temp = 0; 
    if(bms_dev->pack_fCurrent > 0)  return;         /* 充电状态不处理 */
    
    fdl -= bms_dev->pack_fCurrent * 1.0 / 3600;     /* 计算1s中在当前电流下的放电量，时间转换位小时 */

    if(fdl >= BMS_PACK_CAPACITY)                    /*累积放电量大于等于总容量，则认为放电完成一轮*/
    {
        fdl = 0;
        bms_dev->pack_cfd_lc++;
    }
}

/*soh估算*/
void bms_SOH_estimate(bms_device_t *bms_dev)
{
    static float real_dl = 0;       //实际电池电量记录
    static uint8_t startFlag = 0;   /* 0:未开始计算 1:充电 2:放电 */

    if(bms_dev->packSOC == 100)
    {
        startFlag = 2; 
    }
    else  if(bms_dev->packSOC == 4)
    {
        startFlag = 1;
    }
    else{;}
    
    if(!startFlag)  return;

    if(startFlag == 1)                  //充电过程估算
    {
        if(bms_dev->pack_fCurrent < 0)
        {
            real_dl = 0;
            startFlag = 0;
        }
        else
        {
            real_dl += bms_dev->pack_fCurrent * 1.0 /3600;
            if(bms_dev->packSOC == 100)
            {
                bms_dev->pack_SOH_estimate = real_dl / BMS_PACK_CAPACITY * 100;
                real_dl = 0;
                startFlag = 0;
            }
        }
    }
    else if(startFlag == 2)             //放电过程估算
    {
        if(bms_dev->pack_fCurrent > 0)
        {
            real_dl = 0;
            startFlag = 0;
        }
        else
        {
            real_dl -= bms_dev->pack_fCurrent * 1.0 /3600;
            if(bms_dev->packSOC == 0)
            {
                bms_dev->pack_SOH_estimate = real_dl / BMS_PACK_CAPACITY * 100;
                real_dl = 0;
                startFlag = 0;
            }
        }
    }
    else{;}

}


/*ic面积计算*/
void bms_cfd_ICArea(bms_device_t *bms_dev)
{
    static float pre_voltage[20] = {0};
    float temp_voltage[20] = {0};
    static float dQ[20]  = {0};
    static float ic[20] = {0};

    if(bms_dev->pack_cfd_zt != 1)
    {
        return;
    }

    for(int i = 0; i < bms_dev->cellNum; i++)
    {
        temp_voltage[i] = (bms_dev->cell_voltage[i] - pre_voltage[i])/1000.0f;
        dQ[i] += bms_dev->pack_fCurrent * 1.0f /3600.0f;
        if(bms_dev->cell_voltage[i] < 3.50f || bms_dev->cell_voltage[i] > 4.00f)
        {
            dQ[i] = 0;
            return;
        }
        if(temp_voltage[i] < 0.01f) return;
        ic[i] = dQ[i]/temp_voltage[i];
    }


}


void bms_device_offline_check(bms_device_t *bms_dev)
{
    if(bms_dev->offline_time < BMS_OFFLINE_TIME)
        bms_dev->offline_time++;

    if(bms_dev->offline_time >= BMS_OFFLINE_TIME)
    {
        bms_dev->status = BMS_OFFLINE;
        bms_dev->process = BMS_READ_ID;
    }
    else
    {
        bms_dev->status = BMS_ONLINE;
    }

}



void bms_data_deal(bms_device_t *bms_dev,uint8_t *buf,uint16_t len)
{
    uint16_t crc,cmpcrc;
    ESP_LOGI(TAG,"Read data********************************!");
    crc = buf[len-1] | buf[len-2]<<8;
    cmpcrc = bms_crc16_calculate(buf,len-2);
    if(crc != cmpcrc)
    {
        ESP_LOGI(TAG,"Read data crc error!");
        return;
    }

    if((BMS_CMD_READ_REG != buf[1])) 
    {
        ESP_LOGI(TAG,"Read data is not reg data!");
        return;
    }


    if(buf[2] == 62)
    {
        bms_dev->packVoltage    = buf[BMS_FRM_PACK_VOLTAGE_IDX]<<8 | buf[BMS_FRM_PACK_VOLTAGE_IDX+1];
        bms_dev->cellNum        = buf[BMS_FRM_CELL_NUM_IDX]<<8 | buf[BMS_FRM_CELL_NUM_IDX+1];
        bms_dev->packSOC        = buf[BMS_FRM_SOC_IDX]<<8 | buf[BMS_FRM_SOC_IDX+1];
        bms_dev->packCapacity   = buf[BMS_FRM_CAPACITY_IDX]<<8 | buf[BMS_FRM_CAPACITY_IDX+1];
        bms_dev->packSOH        = buf[BMS_FRM_SOH_IDX]<<8 | buf[BMS_FRM_SOH_IDX+1];
        bms_dev->packCurrent    = buf[BMS_FRM_CURRENT_IDX]<<8 | buf[BMS_FRM_CURRENT_IDX+1];
        bms_dev->temp_env       = buf[BMS_FRM_TEMP_IDX]<<8 | buf[BMS_FRM_TEMP_IDX+1];
        bms_dev->low_temp_cell  = buf[BMS_FRM_CELL_LOWTEMP_IDX]<<8 | buf[BMS_FRM_CELL_LOWTEMP_IDX+1];
        bms_dev->temp_mos       = buf[BMS_FRM_MOS_TEMP_IDX]<<8 | buf[BMS_FRM_MOS_TEMP_IDX+1];
        memcpy(bms_dev->cell_voltage,&buf[BMS_FRM_CELL1_VOLTAGE_IDX],40);
        bms_data_exchange(bms_dev->cell_voltage,20);
        bms_dev->high_temp_cell = buf[BMS_FRM_CELL_HIGHTEMP_IDX]<<8 | buf[BMS_FRM_CELL_HIGHTEMP_IDX+1];
        bms_dev->pack_cfd_inRes = buf[BMS_FRM_PACK_INRES_IDX]<<8 | buf[BMS_FRM_PACK_INRES_IDX+1];
        ESP_LOGW(TAG,"Read bms reg data success!");

        bms_dev->pack_fVoltage = bms_dev->packVoltage/100.0f;
        bms_dev->pack_fCurrent = bms_dev->packCurrent/100.0f;

        
    }
    else if(buf[2] == BMS_ID_LEN_24)
    {
        if(memcmp(bms_dev->id,&buf[3],BMS_ID_LEN_24) != 0)
        {
            memcpy(bms_dev->id,&buf[3],BMS_ID_LEN_24);
            // ld_model->update_use_ifo = YES;
            // process_4g_init = config_top_registration_4g;   
            // registerSendflag = 0;  
            
        }
        bms_dev->process = BMS_READ_REG;
    }
    else if(buf[2] == BMS_ID_LEN_28)
    {
        if(memcmp(bms_dev->id,&buf[3],BMS_ID_LEN_28) != 0)
        {
            memcpy(bms_dev->id,&buf[3],BMS_ID_LEN_28);

           
        }
         bms_dev->process = BMS_READ_REG;
    }
    else{/*not deal*/;}
   
    bms_dev->offline_time   = 0;
    // bms_dev->status         = BMS_ONLINE;
}

void bms_device_msg_deal_task_handler(void *pvParameters)
{
    bms_device_t *bms_dev = (bms_device_t *)pvParameters;

    while(bms_rx_queue == NULL)
    {
        ESP_LOGI(TAG, "bms_rx_queue is NULL");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while(1)
    {
        bms_msg_t bms_msg;
        xQueueReceive(bms_rx_queue, &bms_msg, portMAX_DELAY);
        bms_data_deal(bms_dev,bms_msg.data,bms_msg.num);
        // for(int i = 0; i < bms_msg.num; i++)
        // {
        //     printf("%02x ",bms_msg.data[i]);
        // }

         printf("\r\n");
    }
}

void bms_device_cmd_send_task_handler(void *pvParameters)
{
    bms_device_t *bms_dev = (bms_device_t *)pvParameters;

    uint16_t time = 0;

    while(1)
    {
        bms_device_offline_check(bms_dev);
        bms_get_cfd_state(bms_dev);
        bms_get_cfd_time(bms_dev);
        bms_get_cfd_C_rate(bms_dev);
        bms_get_cfd_cycle(bms_dev);
        bms_SOH_estimate(bms_dev);
        bms_cfd_ICArea(bms_dev);
        time++;
        time %= 10;
        if(time == 0)   bms_device_result_send(bms_dev);

        switch (bms_dev->process)
        {
        case BMS_READ_ID:
            /* code */
            bms_device_id_read(bms_dev);
            break;
        case BMS_READ_REG: 
            bms_device_reg_data_read(bms_dev);
            break;
        default:
            break;
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}










#include "mqtt_data_interface.h"
#include "config.h"

static const char *TAG = "PRJ_MQTT_DATA_INTERFACE";

#define MQTT_FIFO_SIZE   20
typedef struct _mqtt_fifo
{
    /* data */
    uint8_t  data[MQTT_FIFO_SIZE][1024];
    uint16_t len[MQTT_FIFO_SIZE];
    uint16_t pos;
    uint16_t tail;
}mqtt_fifo_t;

mqtt_fifo_t mqtt_msg_fifo;

void mqtt_push_msg_to_fifo(uint8_t *data, uint16_t len)
{
    memcpy(&mqtt_msg_fifo.data[mqtt_msg_fifo.tail],data,len);
    mqtt_msg_fifo.len[mqtt_msg_fifo.tail] = len;
    mqtt_msg_fifo.tail++;
    mqtt_msg_fifo.tail %= MQTT_FIFO_SIZE;
}

uint16_t mqtt_crc16(uint8_t *data, uint16_t len)
{
    uint16_t crc16 = 0x0000;
    
    while( len-- ) {
        for(uint8_t i=0x80; i!=0; i>>=1) {
            if((crc16 & 0x8000) != 0) {
                crc16 = crc16 << 1;
                crc16 = crc16 ^ 0x1021;
            }
            else {
                crc16 = crc16 << 1;
            }
            if((*data & i) != 0) {
                crc16 = crc16 ^ 0x1021;  //crc16 = crc16 ^ (0x10000 ^ 0x11021)
            }
        }
        data++;
    }
    return crc16;
}

void mqtt_fixed_frame_fill(uint8_t *frm_buf,uint8_t frm_type,uint16_t frm_num,uint16_t data_len)
{
    uint16_t frm_head = MQTT_FRM_HEAD;
    uint16_t frm_tail = MQTT_FRM_TAIL;
    uint16_t frm_len  = MQTT_FRM_LEN_MIN + data_len;
    static  uint16_t heart  = 0;
    memcpy(&frm_buf[MQTT_FRM_HEAD_IDX],&frm_head,sizeof(frm_head));
    memcpy(&frm_buf[MQTT_FRM_XY_VER_IDX],&sys_config.protocol_ver,sizeof(sys_config.protocol_ver));
    memcpy(&frm_buf[MQTT_FRM_SOUR_ADDR_IDX],&sys_config.dev_id,sizeof(sys_config.dev_id));
    memcpy(&frm_buf[MQTT_FRM_DEV_TYPE_IDX],&sys_config.dev_type,sizeof(sys_config.dev_type));
    memcpy(&frm_buf[MQTT_FRM_DEV_VER_IDX],&sys_config.dev_ver,sizeof(sys_config.dev_ver));
    memcpy(&frm_buf[MQTT_FRM_HEART_IDX],&heart,sizeof(heart));
    memcpy(&frm_buf[MQTT_FRM_TYPE_IDX],&frm_type,sizeof(frm_type));
    memcpy(&frm_buf[MQTT_FRM_DATA_LEN_IDX],&data_len,sizeof(data_len));
    memcpy(&frm_buf[MQTT_FRM_DATA_IDX],&sys_config.dev_id,sizeof(sys_config.dev_id));

    memcpy(&frm_buf[frm_len-2],&frm_tail,sizeof(frm_tail));
}

void mqtt_setFrm_ack(uint32_t setFrm_type,uint16_t frm_num)
{
    uint8_t buf[200]={0};
    uint16_t data_len = 5;
    uint16_t frm_len  = MQTT_FRM_LEN_MIN + data_len;
    uint16_t frm_type = 0x0001;
    static  uint16_t heart  = 0;

    mqtt_fixed_frame_fill(buf,frm_type,frm_num,data_len);

    memcpy(&buf[MQTT_FRM_ORDER_IDX],&setFrm_type,sizeof(setFrm_type));
   

    buf[MQTT_FRM_DATA_IDX+4] = 1;
    
    uint16_t crc = mqtt_crc16(&buf[MQTT_FRM_XY_VER_IDX],frm_len-6);
    memcpy(&buf[frm_len-4],&crc,sizeof(crc));
 
    mqtt_push_msg_to_fifo(buf,frm_len);
}

void mqtt_setFrm_deal(uint8_t *data,uint16_t len)
{
    uint32_t setFrm_type = 0;
    uint16_t frm_heart = 0;
    uint32_t dev_id = 0;
    memcpy(&frm_heart,&data[MQTT_FRM_HEART_IDX],sizeof(frm_heart));
    memcpy(&setFrm_type,data[MQTT_FRM_ORDER_IDX],sizeof(setFrm_type));
    memcpy(&dev_id,&data[MQTT_FRM_DATA_IDX],sizeof(dev_id));
    if(setFrm_type > SETFRM_TYPE_LOW_POWER || dev_id != sys_config.dev_id)     return;

    switch (setFrm_type)
    {
    case SETFRM_TYPE_MODEL_SET:
        /* code */
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到模型设置帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_MODEL_SET,frm_heart);
        break;
    case SETFRM_TYPE_MODEL_USE:
        /* code */
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到模型启用帧！");
        ESP_LOGI(TAG,"*********************************************");
        sys_config.model_config.SOH_model_use = data[MQTT_FRM_DATA_IDX+4];
        sys_config.model_config.SOC_model_use = data[MQTT_FRM_DATA_IDX+5];
        sys_config.model_config.RUL_model_use = data[MQTT_FRM_DATA_IDX+6];
        sys_config.model_config.RSK_model_use = data[MQTT_FRM_DATA_IDX+7];

        mqtt_setFrm_ack(SETFRM_TYPE_MODEL_USE,frm_heart);
        break;
    case SETFRM_TYPE_COMM_USE:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到通信接口启用帧！");
        ESP_LOGI(TAG,"*********************************************");
        sys_config.comm_config.RS485_comm_use   = data[MQTT_FRM_DATA_IDX+4];
        sys_config.comm_config.CAN_comm_use     = data[MQTT_FRM_DATA_IDX+5];
        sys_config.comm_config.WIFI_comm_use    = data[MQTT_FRM_DATA_IDX+6];
        sys_config.comm_config.BLUE_comm_use    = data[MQTT_FRM_DATA_IDX+7];
        sys_config.comm_config.DTU_4G_comm_use  = data[MQTT_FRM_DATA_IDX+8];

        mqtt_setFrm_ack(SETFRM_TYPE_COMM_USE,frm_heart);
        break;
    case SETFRM_TYPE_DATA_INPUT:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到数据输入设置帧！");
        ESP_LOGI(TAG,"*********************************************");
        if(data[MQTT_FRM_DATA_IDX+4] == 1)
        {
            sys_config.model_input_source = MODEL_DATA_SOURCE_BMS;
        }
        else if(data[MQTT_FRM_DATA_IDX+5] == 1)
        {
            sys_config.model_input_source = MODEL_DATA_SOURCE_COMM;
        }
        else if(data[MQTT_FRM_DATA_IDX+6] == 1)
        {
            sys_config.model_input_source = MODEL_DATA_SOURCE_SELF;
        }
        else{;}

        mqtt_setFrm_ack(SETFRM_TYPE_DATA_INPUT,frm_heart);
        break;
    case SETFRM_TYPE_DATA_OUTPUT:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到采集数据输出设置帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_DATA_OUTPUT,frm_heart);
        break;
    case SETFRM_TYPE_RESULT_OUTPUT:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到推理数据输出设置帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_RESULT_OUTPUT,frm_heart);
        break;
    case SETFRM_TYPE_DEV_ID_SET:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到设置编号设置帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_DEV_ID_SET,frm_heart);
        break;
    case SETFRM_TYPE_ED_CAPACITY:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到额定容量设置帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_ED_CAPACITY,frm_heart);
        break;
    case SETFRM_TYPE_RESTART:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收设备重启帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_RESTART,frm_heart);
        break;
    case SETFRM_TYPE_UPDATE:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到软件升级帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_UPDATE,frm_heart);
        break;
    case SETFRM_TYPE_CJ_PARAM_SET:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到采集模块参数设置帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_CJ_PARAM_SET,frm_heart);
        break;
    case SETFRM_TYPE_LOW_POWER:
        /* code */
        break;
    default:
        break;
    }
}

void mqtt_queryFrm_deal(uint8_t *data,uint16_t len)
{
    uint32_t setFrm_type = 0;
    uint16_t frm_heart = 0;
    uint32_t dev_id = 0;
    memcpy(&frm_heart,&data[MQTT_FRM_HEART_IDX],sizeof(frm_heart));
    memcpy(&setFrm_type,data[MQTT_FRM_ORDER_IDX],sizeof(setFrm_type));
    memcpy(&dev_id,&data[MQTT_FRM_DATA_IDX],sizeof(dev_id));
    if(setFrm_type > SETFRM_TYPE_LOW_POWER || dev_id != sys_config.dev_id)     return;


    uint8_t buf[200]={0};
    uint16_t frm_head = MQTT_FRM_HEAD;
    uint16_t frm_tail = MQTT_FRM_TAIL;
    uint16_t data_len = 5;
    uint16_t frm_len  = MQTT_FRM_LEN_MIN + data_len;
    uint16_t frm_type = 0x0001;
    static  uint16_t heart  = 0;
    memcpy(&buf[MQTT_FRM_HEAD_IDX],&frm_head,sizeof(frm_head));
    memcpy(&buf[MQTT_FRM_XY_VER_IDX],&sys_config.protocol_ver,sizeof(sys_config.protocol_ver));
    memcpy(&buf[MQTT_FRM_SOUR_ADDR_IDX],&sys_config.dev_id,sizeof(sys_config.dev_id));
    memcpy(&buf[MQTT_FRM_DEV_TYPE_IDX],&sys_config.dev_type,sizeof(sys_config.dev_type));
    memcpy(&buf[MQTT_FRM_DEV_VER_IDX],&sys_config.dev_ver,sizeof(sys_config.dev_ver));
    memcpy(&buf[MQTT_FRM_HEART_IDX],&heart,sizeof(heart));
    memcpy(&buf[MQTT_FRM_TYPE_IDX],&frm_type,sizeof(frm_type));
    memcpy(&buf[MQTT_FRM_ORDER_IDX],&setFrm_type,sizeof(setFrm_type));
    memcpy(&buf[MQTT_FRM_DATA_LEN_IDX],&data_len,sizeof(data_len));
    memcpy(&buf[MQTT_FRM_DATA_IDX],&sys_config.dev_id,sizeof(sys_config.dev_id));



    switch (setFrm_type)
    {
    case QUERYFRM_TYPE_MODEL_SET:
        /* code */
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到模型查询帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_MODEL_SET,frm_heart);
        break;
    case QUERYFRM_TYPE_MODEL_USE:
        /* code */
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到模型启用信息查询帧！");
        ESP_LOGI(TAG,"*********************************************");
        sys_config.model_config.SOH_model_use = data[MQTT_FRM_DATA_IDX+4];
        sys_config.model_config.SOC_model_use = data[MQTT_FRM_DATA_IDX+5];
        sys_config.model_config.RUL_model_use = data[MQTT_FRM_DATA_IDX+6];
        sys_config.model_config.RSK_model_use = data[MQTT_FRM_DATA_IDX+7];

        mqtt_setFrm_ack(SETFRM_TYPE_MODEL_USE,frm_heart);
        break;
    case QUERYFRM_TYPE_COMM_USE:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到通信接口启用信息查询帧！");
        ESP_LOGI(TAG,"*********************************************");
        sys_config.comm_config.RS485_comm_use   = data[MQTT_FRM_DATA_IDX+4];
        sys_config.comm_config.CAN_comm_use     = data[MQTT_FRM_DATA_IDX+5];
        sys_config.comm_config.WIFI_comm_use    = data[MQTT_FRM_DATA_IDX+6];
        sys_config.comm_config.BLUE_comm_use    = data[MQTT_FRM_DATA_IDX+7];
        sys_config.comm_config.DTU_4G_comm_use  = data[MQTT_FRM_DATA_IDX+8];

        mqtt_setFrm_ack(SETFRM_TYPE_COMM_USE,frm_heart);
        break;
    case QUERYFRM_TYPE_DATA_INPUT:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到数据输入查询帧！");
        ESP_LOGI(TAG,"*********************************************");
        if(data[MQTT_FRM_DATA_IDX+4] == 1)
        {
            sys_config.model_input_source = MODEL_DATA_SOURCE_BMS;
        }
        else if(data[MQTT_FRM_DATA_IDX+5] == 1)
        {
            sys_config.model_input_source = MODEL_DATA_SOURCE_COMM;
        }
        else if(data[MQTT_FRM_DATA_IDX+6] == 1)
        {
            sys_config.model_input_source = MODEL_DATA_SOURCE_SELF;
        }
        else{;}

        mqtt_setFrm_ack(SETFRM_TYPE_DATA_INPUT,frm_heart);
        break;
    case QUERYFRM_TYPE_DATA_OUTPUT:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到采集数据输出查询帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_DATA_OUTPUT,frm_heart);
        break;
    case QUERYFRM_TYPE_RESULT_OUTPUT:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到推理数据输出查询帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_RESULT_OUTPUT,frm_heart);
        break;
    case QUERYFRM_TYPE_DEV_ID_SET:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到设置编号查询帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_DEV_ID_SET,frm_heart);
        break;
    case QUERYFRM_TYPE_ED_CAPACITY:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到额定容量查询帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_ED_CAPACITY,frm_heart);
        break;
    case QUERYFRM_TYPE_CJ_PARAM_SET:
        ESP_LOGI(TAG,"*********************************************");
        ESP_LOGI(TAG,"接收到采集模块参数查询帧！");
        ESP_LOGI(TAG,"*********************************************");
        mqtt_setFrm_ack(SETFRM_TYPE_CJ_PARAM_SET,frm_heart);
        break;
    case QUERYFRM_TYPE_LOW_POWER:
        /* code */
        break;
    default:
        break;
    }
}

void mqtt_uploadAckFrm_deal(uint8_t *data,uint16_t len)
{
    uint32_t sub_frm_type = 0;
    memcpy(&sub_frm_type,data[MQTT_FRM_ORDER_IDX],sizeof(sub_frm_type));
    if(sub_frm_type == 0x00000001)
    {

    }
    else if(sub_frm_type == 0x00000002)
    {

    }


}


void mqtt_data_deal(uint8_t *data,uint16_t len)
{
    uint16_t frm_head;
    uint16_t frm_tail;
    uint16_t frm_type;
    uint16_t frm_crc;
    uint16_t cal_crc;
    uint16_t data_len;
    
    if(len <= MQTT_FRM_LEN_MIN) return; 
    /*  帧头校验 */
    memcpy(&frm_head, &data[MQTT_FRM_HEAD_IDX], sizeof(frm_head));
    if(frm_head != 0xaaaa)      return;
    /*  帧长度校验 */
    memcpy(&data_len,&data[MQTT_FRM_DATA_LEN_IDX],sizeof(data_len));
    if(len != (data_len + MQTT_FRM_LEN_MIN)) return;
    /*  帧尾校验 */
    int frm_len = data_len + MQTT_FRM_LEN_MIN;
    memcpy(&frm_tail, &data[frm_len-2], sizeof(frm_tail));
    if(frm_tail != 0x5555)      return;
    /* CRC校验 */
    memcpy(&frm_crc, &data[frm_len-4], sizeof(frm_crc));
    cal_crc = mqtt_crc16(&data[MQTT_FRM_DATA_IDX], data_len);
    if(frm_crc != cal_crc)      return;

    memcpy(&frm_type,&data[MQTT_FRM_TYPE_IDX],sizeof(frm_type));

    switch(frm_type)
    {
        case MQTT_FRM_TYPE_SET:
            mqtt_setFrm_deal(data,len);
            break;
        case MQTT_FRM_TYPE_QUERY:
            mqtt_queryFrm_deal(data,len);
            break;
        case MQTT_FRM_TYPE_UPLOADACK:
            mqtt_uploadAckFrm_deal(data,len);
            break;
        default:
            break;
    }
}





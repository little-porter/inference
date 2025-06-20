#ifndef __MQTT_DATA_INTERFACE_H__
#define __MQTT_DATA_INTERFACE_H__

#include "project_system.h"

#define MQTT_FRM_HEAD               0xAAAA
#define MQTT_FRM_TAIL               0x5555


#define MQTT_FRM_HEAD_IDX           0   //帧头索引
#define MQTT_FRM_XY_VER_IDX         2   //协议版本号索引
#define MQTT_FRM_SOUR_ADDR_IDX      5   //源设备地址索引
#define MQTT_FRM_DEST_ADDR_IDX      9   //目标设备地址索引
#define MQTT_FRM_DEV_TYPE_IDX       13  //设备类型索引
#define MQTT_FRM_DEV_VER_IDX        16  //设备版本号索引
#define MQTT_FRM_HEART_IDX          21  //帧序号索引
#define MQTT_FRM_TYPE_IDX           23  //帧类型索引
#define MQTT_FRM_ORDER_IDX          25  //数据类型
#define MQTT_FRM_DATA_LEN_IDX       29  //数据长度索引
#define MQTT_FRM_DATA_IDX           31  //数据起始索引

#define MQTT_FRM_LEN_MIN            35  //固定帧长度

#define MQTT_FRM_TYPE_SET           1U  //设置帧
#define MQTT_FRM_TYPE_SETACK        2U  //设置应答帧
#define MQTT_FRM_TYPE_QUERY         3U  //请求帧
#define MQTT_FRM_TYPE_QUERYACK      4U  //请求应答帧
#define MQTT_FRM_TYPE_UPLOAD        5U  //上传帧
#define MQTT_FRM_TYPE_UPLOADACK     6U  //上传应答帧


#define SETFRM_TYPE_MODEL_SET       1U  //设置模型
#define SETFRM_TYPE_MODEL_USE       2U  //设置模型使用
#define SETFRM_TYPE_COMM_USE        3U  //设置通讯接口
#define SETFRM_TYPE_DATA_INPUT      4U  //设置数据输入
#define SETFRM_TYPE_DATA_OUTPUT     5U  //设置数据输出
#define SETFRM_TYPE_RESULT_OUTPUT   6U  //推理结果输出
#define SETFRM_TYPE_DEV_ID_SET      7U  //设置设备ID
#define SETFRM_TYPE_ED_CAPACITY     8U  //设置额定容量
#define SETFRM_TYPE_RESTART         9U  //重启
#define SETFRM_TYPE_UPDATE          10U //设备软件升级
#define SETFRM_TYPE_CJ_PARAM_SET    11U //采集模块参数设置
#define SETFRM_TYPE_LOW_POWER       12U //设置低功耗模式

#define QUERYFRM_TYPE_MODEL_SET       1U  //查询模型数据类型
#define QUERYFRM_TYPE_MODEL_USE       2U  //查询模型使用信息
#define QUERYFRM_TYPE_COMM_USE        3U  //查询通讯接口启用信息
#define QUERYFRM_TYPE_DATA_INPUT      4U  //查询数据模型输入
#define QUERYFRM_TYPE_DATA_OUTPUT     5U  //设置数据输出
#define QUERYFRM_TYPE_RESULT_OUTPUT   6U  //查询推理结果输出信息
#define QUERYFRM_TYPE_DEV_ID_SET      7U  //查询设备ID
#define QUERYFRM_TYPE_ED_CAPACITY     8U  //查询额定容量
// #define QUERYFRM_TYPE_RESTART         9U  //重启
// #define QUERYFRM_TYPE_UPDATE          10U //设备软件升级
#define QUERYFRM_TYPE_CJ_PARAM_SET    11U //查询采集模块参数
#define QUERYFRM_TYPE_LOW_POWER       12U //查询低功耗模式

#define MQTT_FIFO_SIZE   20
typedef struct _mqtt_fifo
{
    /* data */
    uint8_t  data[MQTT_FIFO_SIZE][1024];
    uint16_t len[MQTT_FIFO_SIZE];
    uint16_t pos;
    uint16_t tail;
}mqtt_fifo_t;


void mqtt_push_msg_to_fifo(uint8_t *data, uint16_t len);
void mqtt_data_deal(uint8_t *data,uint16_t len);
extern mqtt_fifo_t mqtt_tx_msg_fifo;

#endif

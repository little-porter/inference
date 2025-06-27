#ifndef __BMS_H__
#define __BMS_H__

#include "project_system.h"

#define BMS_PACK_CELL_MAX_NUM           20

#define BMS_FRM_DATA_IDX(x)     (x*2+3)
#define BMS_FRM_PACK_VOLTAGE_IDX    BMS_FRM_DATA_IDX(0)
#define BMS_FRM_CELL_NUM_IDX        BMS_FRM_DATA_IDX(1)
#define BMS_FRM_SOC_IDX             BMS_FRM_DATA_IDX(2)
#define BMS_FRM_CAPACITY_IDX        BMS_FRM_DATA_IDX(3)
#define BMS_FRM_SOH_IDX             BMS_FRM_DATA_IDX(4)
#define BMS_FRM_CURRENT_IDX         BMS_FRM_DATA_IDX(5)
#define BMS_FRM_TEMP_IDX            BMS_FRM_DATA_IDX(6)
#define BMS_FRM_CELL_LOWTEMP_IDX       BMS_FRM_DATA_IDX(7)
#define BMS_FRM_MOS_TEMP_IDX        BMS_FRM_DATA_IDX(8)
#define BMS_FRM_CELL1_VOLTAGE_IDX   BMS_FRM_DATA_IDX(9)
#define BMS_FRM_CELL2_VOLTAGE_IDX   BMS_FRM_DATA_IDX(10)
#define BMS_FRM_CELL3_VOLTAGE_IDX   BMS_FRM_DATA_IDX(11)
#define BMS_FRM_CELL4_VOLTAGE_IDX   BMS_FRM_DATA_IDX(12)
#define BMS_FRM_CELL5_VOLTAGE_IDX   BMS_FRM_DATA_IDX(13)
#define BMS_FRM_CELL6_VOLTAGE_IDX   BMS_FRM_DATA_IDX(14)
#define BMS_FRM_CELL7_VOLTAGE_IDX   BMS_FRM_DATA_IDX(15)
#define BMS_FRM_CELL8_VOLTAGE_IDX   BMS_FRM_DATA_IDX(16)
#define BMS_FRM_CELL9_VOLTAGE_IDX   BMS_FRM_DATA_IDX(17)
#define BMS_FRM_CELL10_VOLTAGE_IDX  BMS_FRM_DATA_IDX(18)
#define BMS_FRM_CELL11_VOLTAGE_IDX  BMS_FRM_DATA_IDX(19)
#define BMS_FRM_CELL12_VOLTAGE_IDX  BMS_FRM_DATA_IDX(20)
#define BMS_FRM_CELL13_VOLTAGE_IDX  BMS_FRM_DATA_IDX(21)
#define BMS_FRM_CELL14_VOLTAGE_IDX  BMS_FRM_DATA_IDX(22)
#define BMS_FRM_CELL15_VOLTAGE_IDX  BMS_FRM_DATA_IDX(23)
#define BMS_FRM_CELL16_VOLTAGE_IDX  BMS_FRM_DATA_IDX(24)
#define BMS_FRM_CELL17_VOLTAGE_IDX  BMS_FRM_DATA_IDX(25)
#define BMS_FRM_CELL18_VOLTAGE_IDX  BMS_FRM_DATA_IDX(26)
#define BMS_FRM_CELL19_VOLTAGE_IDX  BMS_FRM_DATA_IDX(27)
#define BMS_FRM_CELL20_VOLTAGE_IDX  BMS_FRM_DATA_IDX(28)
#define BMS_FRM_CELL_HIGHTEMP_IDX   BMS_FRM_DATA_IDX(29)
#define BMS_FRM_PACK_INRES_IDX      BMS_FRM_DATA_IDX(30)


typedef enum _bms_status
{
    BMS_ONLINE = 0,
    BMS_OFFLINE = 1,
}bms_status_t;

typedef enum _bms_process
{
    BMS_READ_ID = 0,
    BMS_READ_REG = 1,
}bms_process_t;

typedef struct _bms_device
{
    bms_process_t process;
    bms_status_t status;                            //设备状态//0在线，1离线
    uint16_t     offline_time;
    uint16_t     data_update_flag;

    char id[28];                                    //设备ID
    unsigned char cellNum;                          //电池组电芯数量
    uint16_t packVoltage;                           //电池组电压    0.01V   
    uint16_t packSOC;                               //电池组SOC     x%
    uint16_t packCapacity;                          //电池组容量    0.01Ah
    uint16_t packSOH;                               //电池组健康度  x%
    int16_t packCurrent;                            //电池组电流    0.01A
    int16_t temp_env;                               //环境温度      1℃
    int16_t temp_mos;                               //MOS管温度     1℃
    int16_t low_temp_cell;                          //最低温度      1℃
    int16_t high_temp_cell;                         //最高温度      1℃
    uint16_t cell_voltage[BMS_PACK_CELL_MAX_NUM];   //电池组电芯电压0.001V

    float   pack_fVoltage;      //电池组电压
    float   pack_fCurrent;      //电池组电流
    float   pack_cfd_lc;        //充放电轮次
    float   pack_cfd_zt;        //充放电状态
    float   pack_cfd_Crate;      //充放电率
    float   pack_cfd_time;      //充放电时间
    float   pack_cfd_inRes;      //内阻
    float   pack_cfd_ic;        //增量容量
    float   pack_SOH_estimate;  //SOH估计值
    float   pack_icarea;        //IC面积
}bms_device_t;




extern bms_device_t bms_device;

void bms_device_init(bms_device_t *bms_dev);

void bms_device_result_send(uint8_t type,float *result,int len);

#endif  /* __BMS_H__ */

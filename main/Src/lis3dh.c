#include "lis3dh.h"


static const char *TAG = "PRJ_LIS3DH";


#define LIS3DH_I2C_PORT     I2C_NUM_0
#define LIS3DH_I2C_SDA      GPIO_NUM_7
#define LIS3DH_I2C_SCL      GPIO_NUM_6
#define LIS3DH_I2C_FREQ_HZ  100000

#define LIS3DH_I2C_ADDR     0x18

#define LIS3DH_ACK_CHECK    true




#define LIS3DH_INIT_REG_NUM     9
static uint8_t lis3dh_init_list[][2] = {
    {LIS3DH_CTRL_REG1,      0x2f},
    {LIS3DH_CTRL_REG2,      0x09},
    {LIS3DH_CTRL_REG3,      0x40},
    {LIS3DH_CTRL_REG4,      0x00},
    {LIS3DH_CTRL_REG5,      0x00},
    
    {LIS3DH_INT1_CFG,       0x2a},
    {LIS3DH_INT1_THS,       0x02},
    {LIS3DH_INT1_DURATION,  0x02},
    {LIS3DH_CTRL_REG6,      0x42},
    {LIS3DH_FIFO_CTRL_REG,  0x20},
};

lis3dh_device_t lis3dh_device;

void lis3dh_i2c_init(lis3dh_device_t *lis3dh_dev)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = LIS3DH_I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = LIS3DH_I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = LIS3DH_I2C_FREQ_HZ,
    };

    i2c_param_config(lis3dh_dev->i2c_port, &conf);

    i2c_driver_install(lis3dh_dev->i2c_port, conf.mode,0,0,0);
}

esp_err_t lis3dh_write_reg(lis3dh_device_t *lis3dh_dev,uint8_t reg_addr, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    // 创建I2C写命令序列
    int ret = i2c_master_start(cmd);
    if (ret != ESP_OK) goto end;

    // 发送设备地址（写操作）
    ret = i2c_master_write_byte(cmd, (LIS3DH_I2C_ADDR << 1) | I2C_MASTER_WRITE, LIS3DH_ACK_CHECK);
    if (ret != ESP_OK) goto end;

    // 发送寄存器地址
    ret = i2c_master_write_byte(cmd, reg_addr, LIS3DH_ACK_CHECK);
    if (ret != ESP_OK) goto end;

    // 发送要写入的数据
    ret = i2c_master_write_byte(cmd, data, LIS3DH_ACK_CHECK);
    if (ret != ESP_OK) goto end;

    // 停止信号
    ret = i2c_master_stop(cmd);
    if (ret != ESP_OK) goto end;

    // 执行I2C命令序列
    ret = i2c_master_cmd_begin(LIS3DH_I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);

end:
    i2c_cmd_link_delete(cmd); // 删除I2C命令链路
    
    vTaskDelay(pdMS_TO_TICKS(100)); // 可选延迟

    return ret;
}

esp_err_t lis3dh_read_regs(lis3dh_device_t *lis3dh_dev,uint8_t reg_addr,uint8_t *data, uint8_t size)
{
    if (size == 0) {
        return ESP_OK;
    }

    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // 第一步：发送设备地址（写）和寄存器地址
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LIS3DH_I2C_ADDR << 1) | I2C_MASTER_WRITE, LIS3DH_ACK_CHECK); // 设备地址 + 写位
    i2c_master_write_byte(cmd, reg_addr, LIS3DH_ACK_CHECK); // 寄存器地址

    // 发送第一个I2C命令序列
    ret = i2c_master_cmd_begin(LIS3DH_I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
    if (ret != ESP_OK) {
        i2c_cmd_link_delete(cmd);
        return ret;
    }
    i2c_cmd_link_delete(cmd);

    // 第二步：重新开始I2C传输并读取数据
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LIS3DH_I2C_ADDR << 1) | I2C_MASTER_READ, LIS3DH_ACK_CHECK); // 设备地址 + 读位

    // 读取数据
    if (size > 1) {
        i2c_master_read(cmd, data, size - 1, I2C_MASTER_ACK); // 读取除最后一个字节外的所有字节
    }
    i2c_master_read_byte(cmd, data + size - 1, I2C_MASTER_NACK); // 读取最后一个字节并发送NACK
    i2c_master_stop(cmd);

    // 发送第二个I2C命令序列
    ret = i2c_master_cmd_begin(LIS3DH_I2C_PORT, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    // 可选延迟
    vTaskDelay(pdMS_TO_TICKS(100));

    return ret;
}


void lis3dh_driver_init(lis3dh_device_t *lis3dh_dev)
{ 
    lis3dh_dev->i2c_port = LIS3DH_I2C_PORT;
    lis3dh_dev->i2c_addr = LIS3DH_I2C_ADDR;
    lis3dh_i2c_init(lis3dh_dev);

    uint8_t temp = 0xff;
    for(int i = 0; i < LIS3DH_INIT_REG_NUM; i++)
    {
        temp = 0xff;
        while(temp != lis3dh_init_list[i][2])
        {
            lis3dh_write_reg(lis3dh_dev, lis3dh_init_list[i][0], lis3dh_init_list[i][1]);
            lis3dh_read_regs(lis3dh_dev, lis3dh_init_list[i][0], &temp, 1);
        }
        printf("list_reg_%d : %02x init  success\n",lis3dh_init_list[i][0],temp);
    }

    ESP_LOGI(TAG,"lis3dh init success!");
}






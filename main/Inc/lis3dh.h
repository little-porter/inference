#ifndef __LIS3DH_H__
#define __LIS3DH_H__

#include "project_system.h"
#include "driver/i2c.h"

#define LIS3DH_CTRL_REG1      0x20 
#define LIS3DH_CTRL_REG2      0x21
#define LIS3DH_CTRL_REG3      0x22
#define LIS3DH_CTRL_REG4      0x23
#define LIS3DH_CTRL_REG5      0x24
#define LIS3DH_CTRL_REG6      0x25
#define LIS3DH_INT1_CFG           0x30
#define LIS3DH_INT1_THS           0x32
#define LIS3DH_INT1_DURATION      0x33
#define LIS3DH_FIFO_CTRL_REG  0x2E

typedef struct _lis3dh_device
{
    /* data */
    i2c_port_t  i2c_port;
    uint8_t     i2c_addr;
}lis3dh_device_t;

extern lis3dh_device_t lis3dh_device;

void lis3dh_driver_init(lis3dh_device_t *lis3dh_dev);

#endif

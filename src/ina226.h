/*
 * ina226.h
 *
 *  Created on: Feb 20, 2025
 *      Author: bitstream
 */

#ifndef INC_INA226_H_
#define INC_INA226_H_

#include "hal_data.h"
#include "tx_api.h"
#include "bsp_api.h"
#include <math.h>

// INA226默认地址（ADDR引脚接地）
//#define INA226_ADDR                (0x40 << 1) // 7位地址需左移1位
#define INA226_ADDR                0x40

// 寄存器定义
#define INA226_REG_CONFIG          0x00
#define INA226_REG_SHUNTVOLTAGE    0x01
#define INA226_REG_BUSVOLTAGE      0x02
#define INA226_REG_POWER           0x03
#define INA226_REG_CURRENT         0x04
#define INA226_REG_CALIBRATION     0x05
#define INA226_REG_ALERT_LIMIT     0x07
#define INA226_REG_MASK_ENABLE     0x06
#define INA226_REG_MANUFACTURER_ID 0xFE
#define INA226_REG_DIE_ID          0xFF

// 配置参数（根据需求调整）
#define INA226_AVERAGES            0x01    // 1次平均
#define INA226_BUS_CONV_TIME       0x04    // 1.1ms
#define INA226_SHUNT_CONV_TIME     0x04    // 1.1ms
#define INA226_MODE                0x07    // 连续测量模式

// 校准参数（需根据实际硬件计算）
#define SHUNT_RESISTOR_OHMS        0.005     // 分流电阻值81.92mV / MAX_EXPECTED_CURRENT_A / 1000 = 0.00512Ω，其中81.92mV是INA226的满量程值
#define MAX_EXPECTED_CURRENT_A  8.0     // 最大预期电流，最大可测量电流 = 81.92mV / SHUNT_RESISTOR_OHMS / 1000 = 16.384A

// 警报配置位掩码 (Bit Mask)
#define INA226_ALERT_SHUNT_OV      (1 << 15)  // 分流电压过压
#define INA226_ALERT_SHUNT_UV      (1 << 14)  // 分流电压欠压
#define INA226_ALERT_BUS_OV        (1 << 13)  // 总线电压过压
#define INA226_ALERT_BUS_UV        (1 << 12)  // 总线电压欠压
#define INA226_ALERT_POWER_OV      (1 << 11)  // 功率过限
#define INA226_ALERT_CONV_READY    (1 << 10)  // 转换就绪
#define INA226_ALERT_AFF			(1 << 4) 	//警报中断
#define INA226_ALERT_CVRF			(1 << 3) 	//数据转换完成
#define INA226_ALERT_APOL          (1 << 1)   // 警报引脚极性(0=低有效,1=高有效)
#define INA226_ALERT_LATCH         (1 << 0)   // 警报锁存使能


// 函数声明
fsp_err_t INA226_Init(void);
float INA226_ReadBusVoltage(void);
float INA226_ReadShuntVoltage(void);
float INA226_ReadCurrent(void);
float INA226_ReadPower(void);
uint16_t INA226_CheckDeviceID(void);
void INA226Task();
fsp_err_t INA226_SetAlertConfig(uint16_t alert_mask);
fsp_err_t INA226_SetAlertLimit(uint16_t limit_value);
uint16_t INA226_ReadAlertStatus(void);
void INA226_ClearAlertLatch(void);

#endif /* INC_INA226_H_ */

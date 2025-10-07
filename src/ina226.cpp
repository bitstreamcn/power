/*
 * ina226.c
 *
 *  Created on: Feb 20, 2025
 *      Author: bitstream
 */
#include "ina226.h"

#define MS_TO_TICKS(ms)  ((ULONG)(ms) * TX_TIMER_TICKS_PER_SECOND / 1000)
#define INA226_ALERT_TIMEOUT    MS_TO_TICKS(1000)

fsp_err_t i2c_read_register(uint8_t reg, uint8_t *p_data, uint32_t len);
fsp_err_t i2c_write_register(uint8_t reg, uint8_t *p_data, uint32_t len);

static float current_LSB;          // 电流分辨率

//static const float V_RATE = 10.0f / (10 + 20);
static const float V_RATE = 1;

fsp_err_t INA226_Calibration(void);

fsp_err_t INA226_Calibration(void)
{
    uint16_t cal = (uint16_t) (0.00512 / (current_LSB * 1));
    fsp_err_t status = FSP_SUCCESS;
    // 写入校准寄存器
    uint8_t cal_buf[2] =
    { (uint8_t)(cal >> 8), (uint8_t)(cal & 0xFF) }; // 高字节在前
    while (true)
    {
        status = i2c_write_register (INA226_REG_CALIBRATION, cal_buf, 2);
        if (status == FSP_SUCCESS)
        {
            break;
        }
        tx_thread_sleep (100);
    }
    uint8_t rx_buf[2] =
    { 0 };
    status = i2c_read_register (INA226_REG_CALIBRATION, rx_buf, 2);

    if (cal_buf[0] != rx_buf[0] || cal_buf[1] != rx_buf[1])
    {
        //写入校准寄存器失败
        status = FSP_ERR_UNSUPPORTED;
        return status;
    }
    return status;
}

// 初始化INA226
fsp_err_t INA226_Init(void)
{
    fsp_err_t status = FSP_SUCCESS;
    static uint8_t initialized = 0;
    if (initialized)
        return FSP_SUCCESS;

    // 计算电流LSB和校准值
    //current_LSB = MAX_EXPECTED_CURRENT_A / 32768.0;
    current_LSB = 0.001f;//Current_LSB选择了 1 mA

    // 组合配置寄存器值
    uint16_t config = (INA226_AVERAGES << 9) | (INA226_BUS_CONV_TIME << 6) | (INA226_SHUNT_CONV_TIME << 3) | INA226_MODE;

    // 写入配置寄存器
    uint8_t config_buf[2] =
    { (uint8_t)(config >> 8), (uint8_t)(config & 0xFF) }; // 高字节在前
    status = i2c_write_register (INA226_REG_CONFIG, config_buf, 2);

    if (status != FSP_SUCCESS)
    {
        return status;
    }

    status = INA226_Calibration ();
    if (status != FSP_SUCCESS)
    {
        return status;
    }

    uint16_t alert_mask = INA226_ALERT_CONV_READY //转换完成
    ;
    status = INA226_SetAlertConfig (alert_mask);
    if (status != FSP_SUCCESS)
    {
        return status;
    }

    initialized = 1;
    return status;
}

/**
 * @brief  配置警报功能和中断使能
 * @param  alert_mask : 警报配置掩码（可组合多个INA226_ALERT_xxx标志）
 * @retval HAL状态
 */
fsp_err_t INA226_SetAlertConfig(uint16_t alert_mask)
{
    uint8_t tx_buf[2] =
    { (uint8_t)(alert_mask >> 8), (uint8_t)(alert_mask & 0xFF) };
    fsp_err_t status = FSP_SUCCESS;
    status = i2c_write_register (INA226_REG_MASK_ENABLE, tx_buf, 2);
    return status;
}

/**
 * @brief  设置警报触发限值（写入ALERT_LIMIT寄存器）
 * @param  limit_value : 警报阈值（单位取决于配置）
 * @note   分流电压警报限值 = limit_value * 2.5μV
 */
fsp_err_t INA226_SetAlertLimit(uint16_t limit_value)
{
    uint8_t tx_buf[2] =
    { (uint8_t)(limit_value >> 8), (uint8_t)(limit_value & 0xFF) };

    fsp_err_t status = FSP_SUCCESS;
    status = i2c_write_register (INA226_REG_ALERT_LIMIT, tx_buf, 2);

    return status;
}

/**
 * @brief  读取当前警报状态
 * @retval 警报状态掩码（组合INA226_ALERT_xxx标志）
 */
uint16_t INA226_ReadAlertStatus(void)
{
    uint8_t rx_buf[2];
    uint16_t status_flags = 0;

    fsp_err_t status = FSP_SUCCESS;
    status = i2c_read_register (INA226_REG_MASK_ENABLE, rx_buf, 2);

    if (status == FSP_SUCCESS)
    {
        status_flags = (rx_buf[0] << 8) | rx_buf[1];
    }

    return status_flags;  // 仅保留高6位状态标志
}

/**
 * @brief  清除警报锁存状态
 * @note   通过写入1到转换就绪位(CNVRF)来清除锁存
 */
void INA226_ClearAlertLatch(void)
{
    uint16_t config = INA226_ALERT_CONV_READY;
    uint8_t tx_buf[2] =
    { (uint8_t)(config >> 8), (uint8_t)(config & 0xFF) };

    fsp_err_t status = FSP_SUCCESS;
    status = i2c_write_register (INA226_REG_MASK_ENABLE, tx_buf, 2);

    (void) status;
}

// 读取总线电压（单位：V）
float INA226_ReadBusVoltage(void)
{
    uint8_t rx_buf[2];
    uint16_t voltage;

    fsp_err_t status = FSP_SUCCESS;
    status = i2c_read_register (INA226_REG_BUSVOLTAGE, rx_buf, 2);

    if (status == FSP_SUCCESS)
    {
        voltage = (rx_buf[0] << 8) | rx_buf[1]; // 合并为大端格式
        return (float)((voltage/* >> 3*/) * 0.00125);  //为什么要除以8？
    }
    return 0.0f; // 错误处理
}

// 读取电流（单位：A）
float INA226_ReadCurrent(void)
{
    uint8_t rx_buf[2];
    int16_t current;

    fsp_err_t status = FSP_SUCCESS;
    status = i2c_read_register (INA226_REG_CURRENT, rx_buf, 2);

    if (status == FSP_SUCCESS)
    {
        current = (rx_buf[0] << 8) | rx_buf[1]; // 合并为大端格式
        return current * current_LSB;
    }
    return 0.0f; // 错误处理
}

/**
 * @brief  读取功率值（单位：瓦特 W）
 * @retval 功率值（浮点型），若读取失败返回0.0
 */
float INA226_ReadPower(void)
{
    uint8_t rx_buf[2];
    uint16_t power_raw;

    fsp_err_t status = FSP_SUCCESS;
    status = i2c_read_register (INA226_REG_POWER, rx_buf, 2);

    if (status != FSP_SUCCESS)
    {
        return 0.0f;  // 错误处理
    }

    // 合并为大端格式数据
    power_raw = (rx_buf[0] << 8) | rx_buf[1];

    /* 功率计算逻辑
     * 公式：P = Power_Register × Power_LSB
     * 其中 Power_LSB = 25 × Current_LSB
     */
    const float power_lsb = 25.0f * current_LSB;
    return power_raw * power_lsb;
}

// 验证设备ID
uint16_t INA226_CheckDeviceID(void)
{
    uint8_t id_buf[2];
    fsp_err_t status = FSP_SUCCESS;
    status = i2c_read_register (INA226_REG_MANUFACTURER_ID, id_buf, 2);
    if (status == FSP_SUCCESS)
    {
        uint16_t manuID = (id_buf[0] << 8) | id_buf[1];
        return manuID;  // 正确应返回0x5449
    }
    return 0;
}

extern float out_volt;
extern float out_current;
// 在main.c中
void INA226Task()
{

    //读取标志位，判断是否转换完成
    uint16_t alert_status = INA226_ReadAlertStatus ();
    if (alert_status & INA226_ALERT_CVRF)
    {
        //在这里读取数据
        float voltage = INA226_ReadBusVoltage ();
        float current = INA226_ReadCurrent ();

        if (current <= 0)
        {
            //没有返回结果
            (void) current;
            //continue;
        }
        if (current < 0)
        {
            current = -current;
        }
        if (current < 0.001)
        {
            current = (float)0.001;
        }
        //根据分压电阻计算实际电压
        voltage /= V_RATE;

        if (current > 0)
        {
            out_current = current;
        }
        out_volt = voltage;

    }
}


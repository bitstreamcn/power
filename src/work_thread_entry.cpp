#include "work_thread.h"
#include "hal_data.h"
#include <stdbool.h>
#include "ina226.h"
#include <stdio.h>

#define     VOLATE_R_ADDR     0x2E
#define     CURRENT_R_ADDR    0x3E

const float R_Step = 10000 / 128.0f;

float in_volt = 0.0f;
float out_volt = 0.0f;
float out_current = 0.0f;
fsp_err_t i2c_read_register(uint8_t reg, uint8_t *p_data, uint32_t len);
fsp_err_t i2c_write_register(uint8_t reg, uint8_t *p_data, uint32_t len);

extern float out_volt_set_value;
extern bool adj_volt;
extern float out_current_set_value;
extern bool adj_current;

volatile bool ina226_data_ready = false;
void user_irq7_callback(external_irq_callback_args_t *args)
{
    (void) args;
    ina226_data_ready = true;
}

void sci_spi_callback(spi_callback_args_t *args)
{
    (void) args;

}

volatile int i2c_complete = 0;
void sci_i2c_master_callback(i2c_master_callback_args_t *p_args)
{
    if (I2C_MASTER_EVENT_TX_COMPLETE == p_args->event || I2C_MASTER_EVENT_RX_COMPLETE == p_args->event)
    {
        i2c_complete = 1;
    }
    else if (I2C_MASTER_EVENT_ABORTED == p_args->event)
    {
        i2c_complete = -1; // 发生错误也退出
    }
}

volatile bool scan_complete_flag = false;
void adc_callback(adc_callback_args_t *p_args)
{
    //宏将告知编译器回调函数不使用参数 p_args，从而避免编译器发出警告，
    FSP_PARAMETER_NOT_USED(p_args);
    scan_complete_flag = true;
}

#define I2C_TIMEOUT  1000U

/* 示例：向 I2C 从设备写寄存器 */
fsp_err_t i2c_write_register(uint8_t reg, uint8_t *p_data, uint32_t len)
{
    fsp_err_t err;
    uint8_t tx_buf[1 + len];   // reg + data

    /* 组合寄存器地址和数据 */
    tx_buf[0] = reg;
    memcpy (&tx_buf[1], p_data, len);

    for (int i = 0; i < 5; i++)
    {
        /* 置位完成标志 */
        i2c_complete = 0;

        /* 启动写操作 */
        err = R_SCI_I2C_Write (&g_i2c2_ctrl, tx_buf, sizeof(tx_buf), false); // false=发送 STOP
        if (FSP_SUCCESS != err)
        {
            return err;
        }

        /* 等待传输完成 */
        while (!i2c_complete)
        {
            tx_thread_sleep (1);
        }
        if (i2c_complete > 0)
        {
            break;
        }
        tx_thread_sleep (10);
    }

    if (i2c_complete > 0)
    {
        return FSP_SUCCESS;
    }
    else
    {
        return FSP_ERR_ABORTED;
    }
}

/* 示例：从 I2C 从设备读寄存器 */
fsp_err_t i2c_read_register(uint8_t reg, uint8_t *p_data, uint32_t len)
{
    fsp_err_t err;

    for (int i = 0; i < 5; i++)
    {
        i2c_complete = 0;
        /* 先写寄存器地址 */
        err = R_SCI_I2C_Write (&g_i2c2_ctrl, &reg, 1, true); // issue restart
        if (FSP_SUCCESS != err)
        {
            return err;
        }

        while (!i2c_complete)
        {
            tx_thread_sleep (1);
        }
        if (i2c_complete > 0)
        {
            break;
        }
        tx_thread_sleep (10);
    }
    if (i2c_complete < 0)
    {
        return FSP_ERR_ABORTED;
    }

    i2c_complete = 0;
    /* 再读数据 */
    err = R_SCI_I2C_Read (&g_i2c2_ctrl, p_data, len, false);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    while (!i2c_complete)
    {
        tx_thread_sleep (1);
    }

    if (i2c_complete > 0)
    {
        return FSP_SUCCESS;
    }
    else
    {
        return FSP_ERR_ABORTED;
    }
}

/* work_thread entry function */
void work_thread_entry(void)
{
    /* Initialize the I2C module */
    fsp_err_t err = R_SCI_I2C_Open (g_i2c2.p_ctrl, g_i2c2.p_cfg);
    /* Handle any errors. This function should be defined by the user. */
    assert(FSP_SUCCESS == err);

    tx_thread_sleep (10);

    /* 切换到对应设备地址 */
    err = R_SCI_I2C_SlaveAddressSet (&g_i2c2_ctrl, INA226_ADDR, I2C_MASTER_ADDR_MODE_7BIT);
    assert(FSP_SUCCESS == err);

    err = INA226_Init ();
    assert(FSP_SUCCESS == err);
    assert(INA226_CheckDeviceID () == 0x5449);

    err = R_ICU_ExternalIrqOpen(g_external_irq7.p_ctrl, g_external_irq7.p_cfg);
    assert(FSP_SUCCESS == err);
    err = R_ICU_ExternalIrqEnable(g_external_irq7.p_ctrl);
    assert(FSP_SUCCESS == err);

    //设置电阻初始值
    err = R_SCI_I2C_SlaveAddressSet (g_i2c2.p_ctrl, VOLATE_R_ADDR, I2C_MASTER_ADDR_MODE_7BIT);
    assert(FSP_SUCCESS == err);
    uint8_t regsist_v = 127;
    err = i2c_write_register(0x00, &regsist_v, 1);
    assert(FSP_SUCCESS == err);
    regsist_v = 0;
    err = i2c_read_register (0x00, &regsist_v, 1);
    assert(FSP_SUCCESS == err);


    //设置电阻初始值
    uint8_t regsist_c = 30;
    err = R_SCI_I2C_SlaveAddressSet (&g_i2c2_ctrl, CURRENT_R_ADDR, I2C_MASTER_ADDR_MODE_7BIT);
    assert(FSP_SUCCESS == err);
    err = i2c_write_register(0x00, &regsist_c, 1);
    assert(FSP_SUCCESS == err);
    err = i2c_read_register (0x00, &regsist_c, 1);
    assert(FSP_SUCCESS == err);

    /* Initializes the module. */
    err = R_ADC_Open (&g_adc0_ctrl, &g_adc0_cfg);
    /* Handle any errors. This function should be defined by the user. */
    assert(FSP_SUCCESS == err);

    /* Enable channels. */
    err = R_ADC_ScanCfg (&g_adc0_ctrl, &g_adc0_channel_cfg);
    assert(FSP_SUCCESS == err);

    err = R_ADC_ScanStart(&g_adc0_ctrl);
    assert(FSP_SUCCESS == err);

    float rate = 10.0f / (200 + 10.0f);

    ina226_data_ready = true;

    /* TODO: add your own code here */
    while (1)
    {
        if (ina226_data_ready)
        {
            ina226_data_ready = false;
            err = R_SCI_I2C_SlaveAddressSet (&g_i2c2_ctrl, INA226_ADDR, I2C_MASTER_ADDR_MODE_7BIT);
            //读取ina226数据
            if (FSP_SUCCESS == err)
            {
                INA226Task ();
            }
        }

        if (scan_complete_flag)
        {
            scan_complete_flag = false;
            uint16_t adc_data0 = 0;

            err = R_ADC_Read (&g_adc0_ctrl, ADC_CHANNEL_0, &adc_data0);
            if (FSP_SUCCESS == err)
            {
                //通过分压电阻比例计算真实的输入电压
                in_volt = (float) (((adc_data0 / 4095.0) * 3.3) / rate);
            }

            err = R_ADC_ScanStart(&g_adc0_ctrl);
        }

        //检测用户操作事件，并调整电压电阻和电流电阻值
        if (adj_volt)
        {
            if (abs(out_volt - out_volt_set_value) > 0.03)
            {
                err = R_SCI_I2C_SlaveAddressSet (&g_i2c2_ctrl, VOLATE_R_ADDR, I2C_MASTER_ADDR_MODE_7BIT);
                if (FSP_SUCCESS == err)
                {
                    if ((out_volt - out_volt_set_value) > 0)
                    {
                        if (regsist_v < 127)
                        {
                            regsist_v++;
                        }
                        else
                        {
                            adj_volt = false;
                        }
                    }
                    else
                    {
                        if (regsist_v > 10)
                        {
                            regsist_v--;
                        }
                        else
                        {
                            adj_volt = false;
                        }
                    }
                    err = i2c_write_register(0x00, &regsist_v, 1);
                    //assert(FSP_SUCCESS == err);
                    err = i2c_read_register (0x00, &regsist_v, 1);
                    //assert(FSP_SUCCESS == err);
                }
            }
            else
            {
                adj_volt = false;
            }
        }
        else if (adj_current)
        {
            float diff = out_current - out_current_set_value;
            if (abs(diff) > 0.001)
            {
                if (diff > 0)
                {
                    if (regsist_c > 0)
                    {
                        regsist_c--;
                    }
                    else
                    {
                        adj_current = false;
                    }
                }
                else
                {
                    if (regsist_c < 127)
                    {
                        regsist_c++;
                    }
                    else
                    {
                        adj_current = false;
                    }
                }
                err = R_SCI_I2C_SlaveAddressSet (&g_i2c2_ctrl, CURRENT_R_ADDR, I2C_MASTER_ADDR_MODE_7BIT);
                if (FSP_SUCCESS == err)
                {
                    err = i2c_write_register(0x00, &regsist_c, 1);
                    //assert(FSP_SUCCESS == err);
                    err = i2c_read_register (0x00, &regsist_c, 1);
                    //assert(FSP_SUCCESS == err);
                }
            }
            else
            {
                adj_current = false;
            }
        }

        tx_thread_sleep (1);
    }
}

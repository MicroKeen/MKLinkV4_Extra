/**
 * @file soft_i2c_port.c
 * @brief
 * @author xqyjlj (xqyjlj@126.com)
 * @version 0.0
 * @date 2021-05-12
 * @copyright Copyright © 2021-2021 xqyjlj<xqyjlj@126.com>
 * @SPDX-License-Identifier: Apache-2.0
 *
 * ********************************************************************************
 * @par ChangeLog:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2021-05-12 <td>0.0     <td>xqyjlj  <td>内容
 * </table>
 * ********************************************************************************
 */
#include "soft_i2c.h"
#include "board.h"
#include "perf_counter.h"
#include "hpm_gpio_drv.h"
#include "hpm_gpiom_regs.h"
#include "hpm_gpiom_drv.h"
static misaka_soft_i2c_t i2c_obj;

#define PIN_GPIOM_BASE                  HPM_GPIOM
#define PIN_GPIO                        HPM_FGPIO
#define PIN_PORT                        GPIO_OE_GPIOB
#define SWDIO_DIR                       IOC_PAD_PB15
#define PIN_TCK                         IOC_PAD_PB12
#define PIN_TMS                         IOC_PAD_PB11

#define PIN_TMS_DIR_NUM      15
#define PIN_TCK_NUM          12
#define PIN_TMS_NUM          11

static void gpiom_configure_pin_control_setting(uint8_t pin_index)
{
    gpiom_set_pin_controller(PIN_GPIOM_BASE, PIN_PORT, pin_index, gpiom_core0_fast);
    gpiom_enable_pin_visibility(PIN_GPIOM_BASE, PIN_PORT, pin_index, gpiom_core0_fast);
    gpiom_lock_pin(PIN_GPIOM_BASE, PIN_PORT, pin_index);
}

/**
 * @brief 设置sda引脚电平
 * @param  level 0: 低电平 1: 高电平
 */
static void set_sda(uint8_t level)
{
  gpio_write_pin(PIN_GPIO, PIN_PORT, PIN_TMS_DIR_NUM, true);
  if(level & 0x01) {
     gpio_write_pin(PIN_GPIO, PIN_PORT, PIN_TMS_NUM, true);
  }
  else {
     gpio_write_pin(PIN_GPIO, PIN_PORT, PIN_TMS_NUM, false);
  }
  __asm volatile("fence io, io");
}

/**
 * @brief 设置scl引脚电平
 * @param  level 0: 低电平 1: 高电平
 */
static void set_scl(uint8_t level)
{
    gpio_write_pin(PIN_GPIO, PIN_PORT, PIN_TCK_NUM, level);
}

/**
 * @brief 读取sda引脚电平
 * @return 0 @c 低电平
 * @return 1 @c 高电平
 */
static uint8_t get_sda()
{
    gpio_write_pin(PIN_GPIO, PIN_PORT, PIN_TMS_DIR_NUM, false);
    uint32_t sta =  gpio_read_pin(PIN_GPIO, PIN_PORT, PIN_TMS_NUM);
    __asm volatile("fence io, io");
    return sta;
}

/**
 * @brief 获取互斥量，如果为裸机系统，空函数即可
 */
static void mutex_take()
{

}

/**
 * @brief 释放互斥量，如果为裸机系统，空函数即可
 */
static void mutex_release()
{

}

/**
 * @brief 延时us
 * @param  ms               desc
 */
static void i2c_delay_us(uint16_t ms)
{
    board_delay_us(ms);
}

/**
 * @brief 设置sda引脚为输出模式（硬件无上拉时需要添加）
 */
static void set_sda_out()
{
  gpio_write_pin(PIN_GPIO, PIN_PORT, PIN_TMS_DIR_NUM, true);
  HPM_IOC->PAD[PIN_TMS].FUNC_CTL = IOC_PAD_FUNC_CTL_ALT_SELECT_SET(0); /* as gpio*/
  HPM_IOC->PAD[PIN_TMS].PAD_CTL = IOC_PAD_PAD_CTL_PRS_SET(2) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1);
  gpiom_set_pin_controller(HPM_GPIOM, GPIOM_ASSIGN_GPIOB, PIN_TMS_NUM, gpiom_core0_fast);
  gpio_set_pin_output(PIN_GPIO, PIN_PORT, PIN_TMS_NUM);
}

/**
 * @brief 设置sda引脚为输入模式（硬件无上拉时需要添加）
 */
static void set_sda_in()
{
    gpio_write_pin(PIN_GPIO, PIN_PORT, PIN_TMS_DIR_NUM, false);
    HPM_IOC->PAD[PIN_TMS].PAD_CTL =  IOC_PAD_PAD_CTL_OD_SET(1);
    HPM_IOC->PAD[PIN_TMS].FUNC_CTL = IOC_PAD_FUNC_CTL_ALT_SELECT_SET(0);
    gpio_set_pin_input(PIN_GPIO, PIN_PORT, PIN_TMS_NUM);
    gpiom_set_pin_controller(HPM_GPIOM, GPIOM_ASSIGN_GPIOB, PIN_TMS_NUM, gpiom_core0_fast);
}

/**
 * @brief I2C读写错误回调函数
 * @param  ops              i2c设备
 */
void misaka_soft_i2c_error_callback(const misaka_soft_i2c_t *ops)
{

}

misaka_soft_i2c_t *misaka_soft_i2c_port_init()
{
    HPM_IOC->PAD[PIN_TCK].FUNC_CTL = IOC_PAD_FUNC_CTL_ALT_SELECT_SET(0) | IOC_PAD_FUNC_CTL_LOOP_BACK_MASK; /* as gpio*/
    HPM_IOC->PAD[PIN_TMS].FUNC_CTL = IOC_PAD_FUNC_CTL_ALT_SELECT_SET(0); /* as gpio*/
    HPM_IOC->PAD[SWDIO_DIR].FUNC_CTL = IOC_PAD_FUNC_CTL_ALT_SELECT_SET(0);

    gpiom_configure_pin_control_setting(PIN_TCK_NUM);
    gpiom_configure_pin_control_setting(PIN_TMS_NUM);
    gpiom_configure_pin_control_setting(PIN_TMS_DIR_NUM);

    gpio_set_pin_output(PIN_GPIO, PIN_PORT, PIN_TCK_NUM);
    gpio_set_pin_output(PIN_GPIO, PIN_PORT, PIN_TMS_NUM);
    gpio_set_pin_output_with_initial(PIN_GPIO, PIN_PORT, PIN_TMS_DIR_NUM, false);
    HPM_IOC->PAD[PIN_TCK].PAD_CTL = IOC_PAD_PAD_CTL_HYS_SET(0) | IOC_PAD_PAD_CTL_PRS_SET(1) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1) | IOC_PAD_PAD_CTL_SR_SET(1) | IOC_PAD_PAD_CTL_SPD_SET(3)| IOC_PAD_PAD_CTL_DS_SET(1);
    HPM_IOC->PAD[PIN_TMS].PAD_CTL = IOC_PAD_PAD_CTL_HYS_SET(1) | IOC_PAD_PAD_CTL_PRS_SET(1) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1) | IOC_PAD_PAD_CTL_SR_SET(1) | IOC_PAD_PAD_CTL_SPD_SET(3)| IOC_PAD_PAD_CTL_DS_SET(1);
    HPM_IOC->PAD[SWDIO_DIR].PAD_CTL = IOC_PAD_PAD_CTL_PRS_SET(1) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1) | IOC_PAD_PAD_CTL_SR_SET(1) | IOC_PAD_PAD_CTL_SPD_SET(3)| IOC_PAD_PAD_CTL_DS_SET(0) ;
	
    i2c_obj.delay_us = i2c_delay_us;
    i2c_obj.get_sda = get_sda;
    i2c_obj.mutex_release = mutex_release;
    i2c_obj.mutex_take = mutex_take;
    i2c_obj.set_scl = set_scl;
    i2c_obj.set_sda = set_sda;
    i2c_obj.set_sda_out = set_sda_out;
    i2c_obj.set_sda_in = set_sda_in;
    i2c_obj.us = 10;

    set_sda(1);
    set_scl(1);

    misaka_soft_i2c_init(&i2c_obj);

    return &i2c_obj;
}




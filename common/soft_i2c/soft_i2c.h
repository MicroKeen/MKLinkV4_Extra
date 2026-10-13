/**
 * @file soft_i2c.h
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

#ifndef __MISAKA_SOFT_I2C_H__
#define __MISAKA_SOFT_I2C_H__

#include "stdint.h"

#define misaka_soft_i2c_assert(expr)  ((void)0U)

/*i2c bus operate flag*/
#define MISAKA_SOFT_I2C_WR            0x0000        /**< 写标志位 */
#define MISAKA_SOFT_I2C_RD            (1u << 0)    /**< 读标志位 */
#define MISAKA_SOFT_I2C_ADDR_10BIT    (1u << 2)    /**< 10位地址 */
#define MISAKA_SOFT_I2C_NO_START      (1u << 4)    /**< 无Start信号 */
#define MISAKA_SOFT_I2C_IGNORE_NACK   (1u << 5)    /**< 忽略Nack */
#define MISAKA_SOFT_I2C_NO_READ_ACK   (1u << 6)    /**< 当i2c读取时，不产生Ack */

/* the message of i2c device */
typedef struct
{
	uint16_t addr;                                        /**< 从机地址 */
	uint16_t flags;                                        /**< 读、写标志等 */
	uint32_t len;                                        /**< 读写数据字节数 */
	uint8_t *buf;                                        /**< 读写数据缓冲区指针 */
} misaka_soft_i2c_message, *misaka_soft_i2c_message_t;

struct misaka_soft_i2c_struct
{
	void (*set_sda)(uint8_t state);                        /**< 设置sda引脚电平 */

	void (*set_scl)(uint8_t state);                        /**< 设置scl引脚电平 */

	uint8_t (*get_sda)(void);                            /**< 读取sda引脚电平 */

	void (*delay_us)(uint16_t us);                        /**< 延时us */

	void (*set_sda_out)();                                /**< 设置sda引脚为输出模式（硬件无上拉时需要添加） */

	void (*set_sda_in)();                                /**< 设置sda引脚为输入模式（硬件无上拉时需要添加） */

	void (*mutex_take)();                                /**< 获取互斥量，如果为裸机系统，空函数即可 */

	void (*mutex_release)();                            /**< 释放互斥量，如果为裸机系统，空函数即可 */

	uint16_t us;                                        /**< us延时单位，决定了此模拟iic的速率 */
};

typedef struct misaka_soft_i2c_struct misaka_soft_i2c_t;

/**
 * @brief 产生起始信号
 * @param ops i2c设备
 */
void misaka_soft_i2c_error_callback(const misaka_soft_i2c_t *ops);

/**
 * @brief 外部操作函数
 * @param ops i2c设备
 * @param msgs 消息对象
 * @param num 消息数量
 * @return uint32_t @c 操作的消息数
 */
uint32_t misaka_soft_i2c_transfer(const misaka_soft_i2c_t *ops, misaka_soft_i2c_message *msgs, uint32_t num);

/**
 * @brief 发送数据
 * @param  ops              i2c设备
 * @param  addr             地址
 * @param  flags            标志
 * @param  buf              待发送数据
 * @param  len              发送数据长度
 * @return uint32_t @c 操作的消息数
 */
uint32_t misaka_soft_i2c_master_send(const misaka_soft_i2c_t *ops, uint16_t addr, uint16_t flags, uint8_t *buf, uint32_t len);

/**
 * @brief 发送数据
 * @param ops i2c设备
 * @param addr 地址
 * @param flags 标志
 * @param buf 待发送数据
 * @param len 发送数据长度
 * @return uint32_t @c 操作的消息数
 */
uint32_t misaka_soft_i2c_master_recv(const misaka_soft_i2c_t *ops, uint16_t addr, uint16_t flags, uint8_t *rxbuf, uint32_t rxlen);

/**
 * @brief 发送数据后接收数据
 * @param ops i2c设备
 * @param addr 地址
 * @param txbuf 待发送数据
 * @param txlen 发送数据长度
 * @param rxbuf 待接收数据
 * @param rxlen 接收数据长度
 * @return uint32_t @c 操作的消息数
 */
uint32_t misaka_soft_i2c_master_send_then_recv(const misaka_soft_i2c_t *ops, uint16_t addr, uint8_t *txbuf, uint32_t txlen, uint8_t *rxbuf, uint32_t rxlen);

/**
 * @brief 发送数据后发送数据
 * @param ops i2c设备
 * @param addr 地址
 * @param txbuf1 待发送数据1
 * @param txlen1 发送数据长度1
 * @param txbuf2 待发送数据2
 * @param txlen2 发送数据长度2
 * @return uint32_t @c 操作的消息数
 */
uint32_t misaka_soft_i2c_master_send_then_send(const misaka_soft_i2c_t *ops, uint16_t addr, uint8_t *txbuf1, uint32_t txlen1, uint8_t *txbuf2, uint32_t txlen2);

/**
 * @brief 初始化函数
 * @param ops i2c设备
 */
void misaka_soft_i2c_init(const misaka_soft_i2c_t *i2c_bus);
extern misaka_soft_i2c_t *misaka_soft_i2c_port_init();
#endif //__MISAKA_SOFT_I2C_H__
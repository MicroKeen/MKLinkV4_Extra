/**
 * @file soft_i2c.c
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

#define LOG_NAME "misaka_soft_i2c"

/**
 * @brief 产生起始信号
 * @param ops i2c设备
 */
static void misaka_soft_i2c_start(const misaka_soft_i2c_t *ops)
{
	ops->set_sda_out();
	ops->set_sda(0);
	ops->delay_us((ops->us + 1) >> 1);
	ops->set_scl(0);
}

/**
 * @brief 重复产生起始信号，一般是连续读写中
 * @param ops i2c设备
 */
static void misaka_soft_i2c_restart(const misaka_soft_i2c_t *ops)
{
	ops->set_sda_out();
	ops->set_sda(1);
	ops->set_scl(1);
	ops->delay_us((ops->us + 1) >> 1);
	ops->set_sda(0);
	ops->delay_us((ops->us + 1) >> 1);
	ops->set_scl(0);
}

/**
 * @brief 产生停止信号
 * @param ops i2c设备
 */
static void misaka_soft_i2c_stop(const misaka_soft_i2c_t *ops)
{
	ops->set_sda_out();

	ops->set_sda(0);
	ops->delay_us((ops->us + 1) >> 1);
	ops->set_scl(1);
	ops->delay_us((ops->us + 1) >> 1);
	ops->set_sda(1);
	ops->delay_us(ops->us);
}

/**
 * @brief 等待应答信号到来
 * @param ops i2c设备
 * @return 0 @c 接收应答失败
 * @return 1 @c 接收应答成功
 */
static uint8_t misaka_soft_i2c_wait_ack(const misaka_soft_i2c_t *ops)
{
	uint8_t ack;

	ops->set_sda_in();

	ops->set_sda(1);
	ops->delay_us((ops->us + 1) >> 1);
	ops->set_scl(1);
	ops->delay_us((ops->us + 1) >> 1);
	ack = !ops->get_sda();
	ops->set_scl(0);

	return ack;
}

/**
 * @brief 发送一个字节
 * @param ops i2c设备
 * @param data 待发送数据
 * @return 0 @c 无应答
 * @return 1 @c 有应答
 */
static uint8_t MISAKA_SOFT_I2C_WRite_byte(const misaka_soft_i2c_t *ops, uint8_t data)
{
	int8_t i;
	uint8_t bit;

	ops->set_sda_out();

	for (i = 7; i >= 0; i--)
	{
		ops->set_scl(0);
		bit = (data >> i) & 1;
		ops->set_sda(bit);
		ops->delay_us((ops->us + 1) >> 1);
		ops->set_scl(1);
		ops->delay_us((ops->us + 1) >> 1);
	}
	ops->set_scl(0);
	ops->delay_us((ops->us + 1) >> 1);

	return misaka_soft_i2c_wait_ack(ops);
}

/**
 * @brief 读1个字节
 * @param ops i2c设备
 * @return uint8_t @c 读取到的1个字节
 */
static uint8_t misaka_soft_i2c_read_byte(const misaka_soft_i2c_t *ops)
{
	uint8_t i;
	uint8_t data = 0;

	ops->set_sda_in();

	ops->set_sda(1);
	ops->delay_us((ops->us + 1) >> 1);
	for (i = 0; i < 8; i++)
	{
		data <<= 1;

		ops->set_scl(1);
		ops->delay_us((ops->us + 1) >> 1);
		if (ops->get_sda())
		{
			data |= 1;
		}
		
		ops->set_scl(0);
		ops->delay_us((ops->us + 1) >> 1);
//		ops->delay_us((ops->us));
	}

	return data;
}

/**
 * @brief 发送多字节
 * @param ops i2c设备
 * @param msg 消息对象
 * @return uint32_t @c 发送的字节数
 */
static uint32_t misaka_soft_i2c_send_bytes(const misaka_soft_i2c_t *ops, misaka_soft_i2c_message_t msg)
{
	uint8_t ret;
	uint32_t bytes = 0;
	uint8_t *ptr = msg->buf;
	uint32_t len = msg->len;
	uint16_t ignore_nack = msg->flags & MISAKA_SOFT_I2C_IGNORE_NACK;

	while (len > 0)
	{
		ret = MISAKA_SOFT_I2C_WRite_byte(ops, *ptr);

		if ((ret > 0) || (ignore_nack && (ret == 0)))
		{
			len--;
			ptr++;
			bytes++;
		}
		else if (ret == 0)
		{
			return 0;
		}
		else
		{
			return ret;
		}
	}

	return bytes;
}

/**
 * @brief 产生ACK应答
 * @param ops i2c设备
 * @param ack 0: 不产生 1: 产生
 */
static void misaka_soft_i2c_send_ack_or_nack(const misaka_soft_i2c_t *ops, uint8_t ack)
{
	ops->set_sda_out();

	if (ack)
	{
		ops->set_sda(0);
	}
	ops->delay_us((ops->us + 1) >> 1);
	ops->set_scl(1);
        ops->delay_us((ops->us + 1) >> 1);
	ops->set_scl(0);
}

/**
 * @brief 接收多字节
 * @param ops i2c设备
 * @param msg 消息对象
 * @return uint32_t @c 接收多字节数
 */
static uint32_t misaka_soft_i2c_recv_bytes(const misaka_soft_i2c_t *ops, misaka_soft_i2c_message_t msg)
{
	uint8_t val;
	uint32_t bytes = 0;
	uint8_t *ptr = msg->buf;
	uint32_t len = msg->len;
	uint16_t flags = msg->flags;

	while (len > 0)
	{
		val = misaka_soft_i2c_read_byte(ops);
		if (val >= 0)
		{
			*ptr = val;
			bytes++;
		}
		else
		{
			break;
		}

		ptr++;
		len--;

		if (!(flags & MISAKA_SOFT_I2C_NO_READ_ACK))
		{
			misaka_soft_i2c_send_ack_or_nack(ops, len);
		}
	}

	return bytes;
}

/**
 * @brief 发送地址，启动i2c总线，可重复
 * @param ops i2c设备
 * @param addr 地址
 * @param retries 重复次数
 * @return 0 @c 有应答
 * @return 1 @c 无应答
 */
static uint8_t misaka_soft_i2c_send_address(const misaka_soft_i2c_t *ops, uint8_t addr, uint8_t retries)
{
	uint8_t i;
	uint8_t ret = 0;

	for (i = 0; i <= retries; i++)
	{
		ret = MISAKA_SOFT_I2C_WRite_byte(ops, addr);
		if (ret == 1 || i == retries)
		{
			break;
		}
		misaka_soft_i2c_stop(ops);
		ops->delay_us(ops->us);
		misaka_soft_i2c_start(ops);
	}

	return ret;
}

/**
 * @brief 发送i2c从地址
 * @param ops i2c设备
 * @param msg 消息对象
 * @return 0 @c 有应答
 * @return 1 @c 无应答
 */
static uint8_t misaka_soft_i2c_bit_send_address(const misaka_soft_i2c_t *ops, misaka_soft_i2c_message_t msg)
{
	uint16_t flags = msg->flags;
	uint16_t ignore_nack = msg->flags & MISAKA_SOFT_I2C_IGNORE_NACK;

	uint8_t addr1, addr2, retries;
	uint8_t ret;

	retries = ignore_nack ? 0 : 1;

	if (flags & MISAKA_SOFT_I2C_ADDR_10BIT)
	{
		addr1 = 0xf0 | ((msg->addr >> 7) & 0x06);
		addr2 = msg->addr & 0xff;

		ret = misaka_soft_i2c_send_address(ops, addr1, retries);
		if ((ret != 1) && !ignore_nack)
		{
			return 1;
		}

		ret = MISAKA_SOFT_I2C_WRite_byte(ops, addr2);
		if ((ret != 1) && !ignore_nack)
		{
			return 1;
		}
		if (flags & MISAKA_SOFT_I2C_RD)
		{
			misaka_soft_i2c_restart(ops);
			addr1 |= 0x01;
			ret = misaka_soft_i2c_send_address(ops, addr1, retries);
			if ((ret != 1) && !ignore_nack)
			{
				return 1;
			}
		}
	}
	else
	{
		/* 7-bit addr */
		addr1 = msg->addr << 1;
		if (flags & MISAKA_SOFT_I2C_RD)
		{
			addr1 |= 1;
		}
		ret = misaka_soft_i2c_send_address(ops, addr1, retries);
		if ((ret != 1) && !ignore_nack)
		{
			return 1;
		}
	}

	return 0;
}

/**
 * @brief 内部操作函数
 * @param ops i2c设备
 * @param msgs 消息对象
 * @param num 消息数量
 * @return uint32_t @c 操作的消息数
 */
static uint32_t misaka_soft_i2c_bit_xfer(const misaka_soft_i2c_t *ops, misaka_soft_i2c_message *msgs, uint16_t num)
{
	misaka_soft_i2c_message_t msg;
	uint32_t i, ret;
	uint16_t ignore_nack;

	misaka_soft_i2c_start(ops);
	for (i = 0; i < num; i++)
	{
		msg = &msgs[i];

		ignore_nack = msg->flags & MISAKA_SOFT_I2C_IGNORE_NACK;

		if (!(msg->flags & MISAKA_SOFT_I2C_NO_START))
		{
			if (i)
			{
				misaka_soft_i2c_restart(ops);
			}
			ret = misaka_soft_i2c_bit_send_address(ops, msg);
			if ((ret != 0) && !ignore_nack)
			{
				goto out;
			}
		}
		if (msg->flags & MISAKA_SOFT_I2C_RD)
		{
			ret = misaka_soft_i2c_recv_bytes(ops, msg);
			if (ret < msg->len)
			{
				if (ret >= 0)
				{
					ret = 0;
				}
				goto out;
			}
		}
		else
		{
			ret = misaka_soft_i2c_send_bytes(ops, msg);
			if (ret < msg->len)
			{
				if (ret >= 0)
				{
					ret = 0;
				}
				goto out;
			}
		}
	}
	ret = i;

	out:
	misaka_soft_i2c_stop(ops);

	return ret;
}

/**
 * @brief 外部操作函数
 * @param ops i2c设备
 * @param msgs 消息对象
 * @param num 消息数量
 * @return uint16_t @c 操作的消息数
 */
uint32_t misaka_soft_i2c_transfer(const misaka_soft_i2c_t *ops, misaka_soft_i2c_message *msgs, uint32_t num)
{
	uint32_t ret;

	ops->mutex_take();
	ret = misaka_soft_i2c_bit_xfer(ops, msgs, num);
	ops->mutex_release();
	if (ret != num)
	{
		misaka_soft_i2c_error_callback(ops);
	}

	return ret;
}

/**
 * @brief 发送数据
 * @param ops i2c设备
 * @param addr 地址
 * @param flags 标志
 * @param buf 待发送数据
 * @param len 发送数据长度
 * @return uint32_t @c 操作的消息数
 */
uint32_t misaka_soft_i2c_master_send(const misaka_soft_i2c_t *ops, uint16_t addr, uint16_t flags, uint8_t *buf, uint32_t len)
{
	uint32_t ret;
	misaka_soft_i2c_message msg;

	msg.addr = addr;
	msg.flags = flags;
	msg.len = len;
	msg.buf = (uint8_t *) buf;

	ret = misaka_soft_i2c_transfer(ops, &msg, 1);

	return ret;
}

/**
 * @brief 接收数据
 * @param ops i2c设备
 * @param addr 地址
 * @param flags 标志
 * @param rxbuf 待接收数据
 * @param rxlen 接收数据长度
 * @return uint32_t @c 操作的消息数
 */
uint32_t misaka_soft_i2c_master_recv(const misaka_soft_i2c_t *ops, uint16_t addr, uint16_t flags, uint8_t *rxbuf, uint32_t rxlen)
{
	uint32_t ret;
	misaka_soft_i2c_message msg;

	msg.addr = addr;
	msg.flags = flags | MISAKA_SOFT_I2C_RD;
	msg.len = rxlen;
	msg.buf = rxbuf;

	ret = misaka_soft_i2c_transfer(ops, &msg, 1);

	return ret;
}

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
uint32_t misaka_soft_i2c_master_send_then_recv(const misaka_soft_i2c_t *ops, uint16_t addr, uint8_t *txbuf, uint32_t txlen, uint8_t *rxbuf, uint32_t rxlen)
{
	uint32_t ret;
	misaka_soft_i2c_message msg[2];

	msg[0].addr = addr;
	msg[0].flags = MISAKA_SOFT_I2C_WR;
	msg[0].buf = txbuf;
	msg[0].len = txlen;

	msg[1].addr = addr;
	msg[1].flags = MISAKA_SOFT_I2C_RD;
	msg[1].buf = rxbuf;
	msg[1].len = rxlen;

	ret = misaka_soft_i2c_transfer(ops, msg, 2);

	return ret;
}

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
uint32_t misaka_soft_i2c_master_send_then_send(const misaka_soft_i2c_t *ops, uint16_t addr, uint8_t *txbuf1, uint32_t txlen1, uint8_t *txbuf2, uint32_t txlen2)
{
	uint32_t ret;
	misaka_soft_i2c_message msg[2];

	msg[0].addr = addr;
	msg[0].flags = MISAKA_SOFT_I2C_WR;
	msg[0].buf = txbuf1;
	msg[0].len = txlen1;

	msg[1].addr = addr;
	msg[1].flags = MISAKA_SOFT_I2C_WR | MISAKA_SOFT_I2C_NO_START;
	msg[1].buf = txbuf2;
	msg[1].len = txlen2;

	ret = misaka_soft_i2c_transfer(ops, msg, 2);

	return ret;
}

/**
 * @brief 初始化函数
 * @param ops i2c设备
 */
void misaka_soft_i2c_init(const misaka_soft_i2c_t *ops)
{
	misaka_soft_i2c_assert(ops);
	misaka_soft_i2c_assert(ops->delay_us);
	misaka_soft_i2c_assert(ops->set_scl);
	misaka_soft_i2c_assert(ops->set_sda);
	misaka_soft_i2c_assert(ops->set_sda_out);
	misaka_soft_i2c_assert(ops->get_sda);
	misaka_soft_i2c_assert(ops->set_sda_in);
	misaka_soft_i2c_assert(ops->mutex_release);
	misaka_soft_i2c_assert(ops->mutex_take);
}
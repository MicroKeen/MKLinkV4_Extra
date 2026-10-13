/**
 * @file key_driver.c
 * @author Jason Wen (embediot@163.com)
 * @brief 按键模块的驱动层，用来适配不同的单片机引脚
 * @version 0.1
 * @date 2024-06-3
 * 
 * @copyright EmbedIoT Studio Copyright (c) 2024
 * 
 */
#include "board.h"
#include "hpm_gpio_drv.h"
#include "key_driver.h"


//按键ID与引脚绑定结构体
typedef struct key_pinmap
{
    key_id_t key_id;
    GPIO_Type *ptr;
    uint32_t port;
    unsigned short pin;
    uint8_t detect_level;  //按键按下时的电平状态，0-低电平，1-高电平
}key_pinmap_t;

key_pinmap_t key_pinmap[KEY_ID_MAX] = 
{
    {.key_id = KEY_ID_0, .ptr = HPM_GPIO0,.port = GPIO_DI_GPIOA, .pin = 31,.detect_level = 0},    //K1
};

static void _init(void)
{
    //在这里初始化GPIO引脚
    HPM_IOC->PAD[31].PAD_CTL =  IOC_PAD_PAD_CTL_PRS_SET(2) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1);
    gpio_set_pin_input(HPM_GPIO0, GPIO_DI_GPIOA, 31);
}

static bool _read_pin_state(key_id_t key_id)
{
    //在这里返回按键引脚电平
    int i = 0;
    unsigned char state;

    for(i = 0;i < KEY_ID_MAX;i++){
        if(key_id == key_pinmap[i].key_id)
        state = gpio_read_pin(key_pinmap[i].ptr, key_pinmap[i].port, key_pinmap[i].pin);
    }

    if(key_pinmap[key_id].detect_level)return ((state == 1) ? true : false);
    else return ((state == 1) ? false : true);
}


key_driver_t key_driver = 
{
    .init = _init,
    .read_pin_state = _read_pin_state,
};




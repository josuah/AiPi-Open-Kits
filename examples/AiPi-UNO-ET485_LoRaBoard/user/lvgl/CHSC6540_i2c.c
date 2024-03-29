/**
 * @file CHSC6540_i2c.c
 * @brief
 *
 * Copyright (c) 2021 Bouffalolab team
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 */

#include "touch.h"

#ifdef USER_TOUCH_I2C_CHSC6540
#include "bflb_mtimer.h"
#include "bflb_gpio.h"
#include "bflb_i2c.h"
#include "CHSC6540_i2c.h"
#include "chsc6x_main.h"

static struct bflb_device_s* touch_chsc6540_i2c = NULL;
static struct bflb_device_s* touch_gpio = NULL;


static touch_coord_t touch_coord_chsc6540 = { 0 };
static uint8_t u8_point_num;

void touch_int_gpio_isr(uint8_t pin)
{
    static uint32_t i = 0;
    if (pin == TOUCH_INT_PIN) {
        // printf("gpio_isr i:%d\r\n", i++);
        chsc6x_read_touch_info(&u8_point_num, &touch_coord_chsc6540, TLSC6X_MAX_TOUCH_POINT);
    }
}

static void i2c_gpio_init(void)
{
    struct bflb_device_s* i2c_gpio = NULL;
    i2c_gpio = bflb_device_get_by_name("gpio");
    touch_gpio = bflb_device_get_by_name("gpio");

    /* I2C0_SCL */
    bflb_gpio_init(i2c_gpio, TOUCH_I2C_SCL_PIN, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    /* I2C0_SDA */
    bflb_gpio_init(i2c_gpio, TOUCH_I2C_SDA_PIN, GPIO_FUNC_I2C0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);

    // bflb_gpio_init(touch_gpio, TOUCH_RESET_PIN, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_2);
    // bflb_gpio_set(i2c_gpio, TOUCH_RESET_PIN);
    bflb_irq_disable(touch_gpio->irq_num);
    bflb_gpio_init(touch_gpio, TOUCH_INT_PIN, GPIO_INPUT | GPIO_PULLUP | GPIO_SMT_EN);
    bflb_gpio_int_init(touch_gpio, TOUCH_INT_PIN, GPIO_INT_TRIG_MODE_SYNC_FALLING_EDGE);
    bflb_gpio_irq_attach(TOUCH_INT_PIN, touch_int_gpio_isr);
    bflb_irq_enable(touch_gpio->irq_num);
}

void chsc6540_reset_pin_ctrl(uint8_t level)
{
    if (level) {
        bflb_gpio_set(touch_gpio, TOUCH_RESET_PIN);
    }
    else {
        bflb_gpio_reset(touch_gpio, TOUCH_RESET_PIN);
    }

}

static int chsc6540_i2c_peripheral_init(void)
{
    touch_chsc6540_i2c = bflb_device_get_by_name("i2c0");

    if (touch_chsc6540_i2c) {
        // printf("ft6x36 i2c gpio init\r\n");
        /* init i2c gpio */
        i2c_gpio_init();
        /* init i2c 200k */
        bflb_i2c_init(touch_chsc6540_i2c, 200000);
    }
    else {
        printf("i2c device get fail\r\n");
        return -1;
    }

    return 0;
}

// static 
int chsc6540_i2c_read_byte(uint8_t* register_addr, uint8_t addr_len, uint8_t* data_buf, uint16_t len)
{
    static struct bflb_i2c_msg_s msg[2];

    msg[0].addr = TLSC6X_IIC_ADDR;
    msg[0].flags = I2C_M_NOSTOP;
    msg[0].buffer = register_addr;
    msg[0].length = addr_len;

    msg[1].addr = TLSC6X_IIC_ADDR;
    msg[1].flags = I2C_M_READ;
    msg[1].buffer = data_buf;
    msg[1].length = len;
    return bflb_i2c_transfer(touch_chsc6540_i2c, msg, 2);

    // return 0;
}
// static 
int chsc6540_i2c_write_byte(uint8_t* register_addr, uint8_t addr_len, uint8_t* data_buf, uint16_t len)
{
    static struct bflb_i2c_msg_s msg[2];

    msg[0].addr = TLSC6X_IIC_ADDR;
    msg[0].flags = I2C_M_NOSTOP;
    msg[0].buffer = register_addr;
    msg[0].length = addr_len;

    msg[1].addr = TLSC6X_IIC_ADDR;
    msg[1].flags = 0;
    msg[1].buffer = data_buf;
    msg[1].length = len;

    return bflb_i2c_transfer(touch_chsc6540_i2c, msg, 2);
    // return 0;
}

int chsc6540_get_gesture_id()
{
    uint8_t data_buf = 0;

    // if (chsc6540_i2c_read_byte(TLSC6X_REG_VENDOR_ID_ADDR, 1, &data_buf, 1)) {
    //     return -1;
    // }

    return data_buf;
}

int chsc6540_i2c_init(touch_coord_t* max_value)
{
    uint8_t data_buf = 0;
    printf("chsc6540 i2c init\r\n");

    chsc6540_i2c_peripheral_init();

    // if (chsc6540_i2c_read_byte(TLSC6X_REG_VENDOR_ID_ADDR, &data_buf, 1)) {
    //     return -1;
    // }
    // printf("Touch Device ID: 0x%02x\r\n", data_buf);

    // if (chsc6540_i2c_read_byte(TLSC6X_REG_IC_YTPE_ADDR, &data_buf, 1)) {
    //     return -1;
    // }
    // printf("Touch Chip ID: 0x%02x\r\n", data_buf);

    // chsc6x_init();


    return 0;
}

#if 1
int chsc6540_i2c_read(uint8_t* point_num, touch_coord_t* touch_coord, uint8_t max_num)
{
    *point_num = 0;

    if (point_num == NULL || touch_coord == NULL || max_num == 0) {
        return -1;
    }

    if (u8_point_num > 0) {
        *point_num = u8_point_num;
        u8_point_num = 0;
        touch_coord->coord_x = touch_coord_chsc6540.coord_x;
        touch_coord->coord_y = touch_coord_chsc6540.coord_y;
    }

    return 0;

}
#else
int chsc6540_i2c_read(uint8_t* point_num, touch_coord_t* touch_coord, uint8_t max_num)
{
    uint8_t point_data[TLSC6X_MAX_TOUCH_POINT * TLSC6X_DATA_CNT_FOR_POINT + 1] = { 0 };
    uint8_t touch_num = 0, tid, touch_info;
    static uint8_t fresh_comm = 0;
    uint16_t tp_dev_x, tp_dev_y, touchSize;
    uint8_t x_h = 0, y_h = 0;
    uint16_t x2 = 0, y2 = 0;
    int i, ret;
    uint8_t addr_buf[2];

    addr_buf[0] = TLSC6X_REG_READ_COOR_ADDR;
    *point_num = 0;

    if (point_num == NULL || touch_coord == NULL || max_num == 0) {
        return -1;
    }

    /* Get the first point */
    ret = chsc6540_i2c_read_byte(addr_buf, 1, point_data, (TLSC6X_MAX_TOUCH_POINT * TLSC6X_DATA_CNT_FOR_POINT));
    if (ret != 0) {
        printf("chsc6540_i2c_read_byte error:%d\r\n", ret);
    }
    printf("IIC read:");
    for (i = 0; i<(TLSC6X_MAX_TOUCH_POINT * TLSC6X_DATA_CNT_FOR_POINT); i++) {
        printf("%d ", point_data[i]);
    }
    printf("\r\n");
    y_h = (point_data[0]>>8);
    x_h = (point_data[0]>>7);
    //		printf("y_h:%d, x_h:%d \n",y_h,x_h);
    if (x_h == 1)
        y2 = 256 + point_data[2];
    else
        y2 = point_data[2];

    if (y_h == 1) {
        x2 = 256 + point_data[1];
        //			printf("		x2:%d			\n",x2);
    }
    else
        x2 = point_data[1];

    if (TP_LEVEL) {
        tp_dev_x = y2;
        tp_dev_y = x2;
    }
    else {
        tp_dev_x = x2;
        tp_dev_y = y2;
    }
    if (tp_dev_x > TP_MAX_X) {
        tp_dev_x = TP_MAX_X;
    }
    if (tp_dev_y > TP_MAX_Y) {
        tp_dev_y = TP_MAX_Y;
    }
    if (X_MIRRORING) {
        tp_dev_x = TP_MAX_X - 1 - tp_dev_x;
    }
    if (Y_MIRRORING) {
        tp_dev_y = TP_MAX_Y - 1 - tp_dev_y;
    }

    touch_coord[0].coord_x = tp_dev_x;
    touch_coord[0].coord_y = tp_dev_y + 35;
    *point_num += 1;
    // printf("touch_panel:(x=%d, y=%d)[%d]\r\n", touch_coord[0].coord_x, touch_coord[0].coord_y, *point_num);

    return 0;
}
#endif

#endif

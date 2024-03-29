#include "chsc6x_platform.h"
#include "chsc6x_main.h"

// #include "bflb_gpio.h"
// #include "bflb_i2c.h"
#include "bflb_mtimer.h"


extern int chsc6540_i2c_read_byte(uint8_t *register_addr, uint8_t addr_len, uint8_t *data_buf, uint16_t len);
extern int chsc6540_i2c_write_byte(uint8_t *register_addr, uint8_t addr_len, uint8_t *data_buf, uint16_t len);
extern void chsc6540_reset_pin_ctrl(uint8_t level);

int semi_touch_i2c_init(void)
{
    // IIC_Init();
    return 0;
}


int chsc6x_read_bytes_u8addr_sub(uint8_t id, uint8_t adr, uint8_t *rxbuf, uint16_t lenth)
{
    uint8_t addr_buf[2];

    addr_buf[0] = adr;
    int ret = chsc6540_i2c_read_byte(addr_buf, 1, rxbuf, lenth);

    if(ret == 0) {
        return OS_OK;
    }else{
        chsc6x_err("chsc6x_read_bytes_u8addr error:%d\r\n", ret);
        return -OS_ERROR;
    }
}

int chsc6x_write_bytes_u16addr_sub(uint8_t id, uint16_t adr, uint8_t *txbuf, uint16_t lenth)
{
    uint8_t addr_buf[2];

    addr_buf[0] = (uint8_t)(adr >> 8);
    addr_buf[1] = (uint8_t)(adr & 0x00ff);
    int ret = chsc6540_i2c_write_byte(addr_buf, 2, txbuf, lenth);
    // int ret = chsc6540_i2c_write_byte((uint8_t *)&adr, 2, txbuf, lenth);
    if(ret == 0) {
        return OS_OK;
    }else{
        chsc6x_err("chsc6x_write_bytes_u16addr error:%d\r\n", ret);
        return -OS_ERROR;
    }
}

int chsc6x_read_bytes_u16addr_sub(uint8_t id, uint16_t adr, uint8_t *rxbuf, uint16_t lenth)
{
    uint8_t addr_buf[2];

    addr_buf[0] = (uint8_t)(adr >> 8);
    addr_buf[1] = (uint8_t)(adr & 0x00ff);
    int ret = chsc6540_i2c_read_byte(addr_buf, 2, rxbuf, lenth);
    // int ret = chsc6540_i2c_read_byte((uint8_t *)&adr, 2, rxbuf, lenth);
    if(ret == 0) {
        return OS_OK;
    }else{
        chsc6x_err("chsc6x_read_bytes_u16addr error:%d\r\n", ret);
        return -OS_ERROR;
    }
}

void chsc6x_msleep(int ms)
{
    bflb_mtimer_delay_ms(ms);
}

void chsc6x_tp_reset_delay(chsc6x_reset_e type)
{
    switch (type)
    {
        case HW_CMD_RESET:
            chsc6x_msleep(30);
            break;

        case HW_ACTIVE_RESET:
            chsc6x_msleep(2);
            break;

        default:
            break;
    }

}

int chsc6x_tp_reset(chsc6x_reset_e type)
{
    if (RESET_NONE >= type || RESET_MAX <= type)
    {
        return -OS_ERROR;
    }
    /*maybe you should disable irq*/
    semi_rst_pin_low(st_dev.rst_pin);
    chsc6x_msleep(20);
    semi_rst_pin_high(st_dev.rst_pin);
    chsc6x_tp_reset_delay(type);

    return OS_OK;
}

/* GPIO-INT*/
// int semi_touch_get_int(void)
// {

//     return SENSOR_INT_PIN;
// }

/* GPIO-RST*/
// int semi_touch_get_rst(void)
// {
//     return TOUCH_RESET_PIN;
// }

void semi_rst_pin_low(int pin)
{
    chsc6540_reset_pin_ctrl(0);
}

void semi_rst_pin_high(int pin)
{
    chsc6540_reset_pin_ctrl(1);
}

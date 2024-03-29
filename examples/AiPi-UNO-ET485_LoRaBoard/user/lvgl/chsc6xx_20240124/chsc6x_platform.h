#ifndef __CHSC6X_PLATFORM_H__
#define __CHSC6X_PLATFORM_H__
//#include "printf.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#if 0
#define chsc6x_info(a, ...) printf("[chsc6x_info]:" a, ##__VA_ARGS__)
#else
#define chsc6x_info       
// #define chsc6x_err         
#endif
#define chsc6x_err(a, ...) printf("[chsc6x_err]:" a, ##__VA_ARGS__)


#define CHSC6X_I2C_ID      (0x5c) //8bit 

#define CHSC6X_MAX_POINTS_NUM     (1)
#define CHSC6X_RES_MAX_X	(370)
#define CHSC6X_RES_MAX_Y	(370)

/*MACRO SWITCH for driver update TP FW */
#define CHSC6X_AUTO_UPGRADE           (1)

/*MACRO SWITCH for TP gesture function */
#define CHSC6X_GESTURE                (0)

/*MACRO SWITCH for multi TP_VENDOR Compatible update TP FW */
#define CHSC6X_MUL_VENDOR_UPGRADE     (0)

#define MAX_IIC_WR_LEN          (32)
#define MAX_IIC_RD_LEN          (32)

typedef enum
{
    RESET_NONE,
    HW_CMD_RESET,
    HW_ACTIVE_RESET,
    RESET_MAX
} chsc6x_reset_e;

/* fail : <0 */
int chsc6x_i2c_read(uint8_t id, uint8_t *p_data, uint16_t lenth);
int chsc6x_i2c_write(uint8_t id,uint8_t *p_data,uint16_t lenth);

/* RETURN:0->pass else->fail */
int chsc6x_read_bytes_u8addr_sub(uint8_t id, uint8_t adr, uint8_t *rxbuf, uint16_t lenth);
/* RETURN:0->pass else->fail */
int chsc6x_read_bytes_u16addr_sub(uint8_t id, uint16_t adr, uint8_t *rxbuf, uint16_t lenth);

/* RETURN:0->pass else->fail */
int chsc6x_write_bytes_u16addr_sub(uint8_t id, uint16_t addr, uint8_t *txbuf, uint16_t lenth);

void chsc6x_msleep(int ms);
int chsc6x_tp_reset(chsc6x_reset_e type);
void semi_rst_pin_low(int pin);
void semi_rst_pin_high(int pin);
int semi_touch_get_int(void);
int semi_touch_get_rst(void);
extern struct sm_touch_dev st_dev;
int semi_touch_i2c_init(void);

#endif

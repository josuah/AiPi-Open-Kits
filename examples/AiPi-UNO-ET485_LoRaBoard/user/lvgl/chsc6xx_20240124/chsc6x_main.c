#include "chsc6x_main.h"
#include "chsc6x_upgrade.h"
#include "chsc6x_platform.h"

struct sm_touch_dev st_dev;
extern uint8_t g_i2c_addr;

int chsc6x_read_touch_info(uint8_t *p_point_num, touch_coord_t *touch_coord, uint8_t max_num)
{
    int ret;
    uint8_t i, point_num;
    uint8_t rd_len = 16;
    uint8_t rd_buf[24] = { 0 };
    struct ts_event events[CHSC6X_MAX_POINTS_NUM];
    memset(&events, 0, sizeof(events));

    ret = chsc6x_read_bytes_u8addr(g_i2c_addr, 0x0, rd_buf, rd_len);

    if( 0 != ret)
    {
        chsc6x_err("chsc6x: chsc6x_read_touch_info iic err! rd_len=%d, ret=%d \r\n", rd_len, ret);
        return -OS_ERROR;
    }
    *p_point_num = 0;
    point_num = rd_buf[2] & 0x07;
    for (i = 0; i < CHSC6X_MAX_POINTS_NUM; i++)
    {
        if ((rd_buf[6 * i + 3] & 0xc0) == 0xc0)
        {
            continue;
        }
        events[i].x = (uint16_t) (rd_buf[6 * i + 3] & 0x0F) << 8 | (uint16_t) rd_buf[6 * i + 4];
        events[i].y= (uint16_t) (rd_buf[6 * i + 5] & 0x0F) << 8 | (uint16_t) rd_buf[6 * i + 6];
        events[i].flag = rd_buf[6 * i + 3] >> 4;
        events[i].id = rd_buf[6 * i + 5] >> 4;

        touch_coord[0].coord_x = events[i].x;
        touch_coord[0].coord_y = events[i].y;
        *p_point_num += 1;
    }
    chsc6x_info("chsc6x: point_num:%d, X1:%d, Y1:%d, flag1:%d, id1:%d; \r\n", \
                point_num, events[0].x, events[0].y, events[0].flag, events[0].id);
    
    return OS_OK;
}


void chsc6x_resume(void)    //Call when the display lights up
{
    chsc6x_info("chsc6x_resume enter.\r\n");
    chsc6x_tp_reset(HW_CMD_RESET);
}

void chsc6x_suspend(void)    //Call when the display is off
{
    chsc6x_info("chsc6x_suspend enter.\r\n");
    uint8_t buff[1] = {0};
    chsc6x_tp_reset(HW_CMD_RESET);
#if CHSC6X_GESTURE
    if(OS_OK != chsc6x_write_bytes_u16addr_sub(g_i2c_addr, 0xD001, buff, 0))
    {
        chsc6x_err("chsc6x: enter gesture failed! \r\n");
    }
#else
    if(OS_OK != chsc6x_write_bytes_u16addr_sub(g_i2c_addr, 0xA503, buff, 0))
    {
        chsc6x_err("chsc6x: enter suspend failed! \r\n");
    }
#endif
}

void chsc6x_init(void)
{
    uint8_t i = 0;
    int ret = 0;
    uint8_t fw_update_ret_flag = 0; //1:update OK, !0 fail
    uint8_t chip_id = 0xFF;
    char probe_flag = 0;
    struct ts_fw_infos fw_infos;
    memset(&st_dev, 0, sizeof(st_dev));
    // st_dev.int_pin = semi_touch_get_int();
    // st_dev.rst_pin = semi_touch_get_rst();
    // semi_rst_pin_high(st_dev.rst_pin);
    // semi_touch_i2c_init();
    // chsc6x_msleep(60);
    // chsc6x_tp_reset(HW_CMD_RESET);

    /*get chip id*/
    chsc6x_get_normal_chip_id(&chip_id);
    if (CHSC6X_I2C_ID == chip_id)
    {
        probe_flag = 1;
    }
    else
    {
        chsc6x_get_boot_chip_id(&chip_id);
        if (CHSC6X_I2C_ID == chip_id)
        {
            probe_flag = 1;
        }
        else
        {
            chsc6x_err("get id:%d failed! exit drivers! \r\n", chip_id);
        }
    }

    for(i = 0; i < TP_RETRY_CNT2; i++)
    {
        ret = chsc6x_tp_dect(&fw_infos, &fw_update_ret_flag);
        if(1 == ret)
        {
            chsc6x_info("chsc6x_tp_dect succeed!\r\n");
        #if CHSC6X_AUTO_UPGRADE /* If need update FW */
            if(1 == fw_update_ret_flag)
            {
                chsc6x_err("update fw succeed! \r\n");
                break;
            }
            else
            {
                chsc6x_err("update fw failed! \r\n");
            }
        #else
            break;
        #endif
        }
        else
        {
            chsc6x_err("chsc6x_tp_dect failed! i = %d \r\n", i);
        }
    }
}

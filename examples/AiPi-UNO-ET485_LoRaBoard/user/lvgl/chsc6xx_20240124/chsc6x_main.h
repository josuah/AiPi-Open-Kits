#ifndef __CHSC6X_MAIN_H__
#define __CHSC6X_MAIN_H__
#include <stdint.h>
#include "touch.h"

#define OS_OK                    (0)
#define OS_ERROR                 (1)
#define TP_RETRY_CNT             (2)
#define TP_RETRY_CNT2            (3)
#define TP_RETRY_CNT3            (5)

typedef struct sm_touch_dev
{    
    int int_pin;
    int rst_pin;
}sm_touch_dev, *psm_touch_dev;

struct ts_event {
     uint16_t x; /*x coordinate */
     uint16_t y; /*y coordinate */
    int flag; /* touch event flag: 0 -- down; 4-- up; 8 -- contact */
    int id;   /*touch ID */
};


void chsc6x_init(void);
int chsc6x_read_touch_info(uint8_t *p_point_num, touch_coord_t *touch_coord, uint8_t max_num);

#endif



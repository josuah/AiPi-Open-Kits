#ifndef __CHSC6X_COMP_H__
#define __CHSC6X_COMP_H__
#include <stdint.h>

#define TXRX_ADDR       (0x9000)
#define CMD_ADDR        (0x9f00)
#define RSP_ADDR        (0x9f40)


struct chsc6x_updfile_header {
    uint32_t sig;
    uint32_t resv;
    uint32_t n_cfg;
    uint32_t n_match;
    uint32_t len_cfg;
    uint32_t len_boot;
};

typedef struct _test_cmd_wr {
    /* offset 0; */
    uint8_t id;    /* cmd_id; */
    uint8_t idv;    /* inverse of cmd_id */
    uint16_t d0;    /* data 0 */
    uint16_t d1;    /* data 1 */
    uint16_t d2;    /* data 2 */
    /* offset 8; */
    uint8_t resv;    /* offset 8 */
    uint8_t tag;    /* offset 9 */
    uint16_t chk;    /* 16 bit checksum */
    uint16_t s2Pad0;    /*  */
    uint16_t s2Pad1;    /*  */
} ctp_tst_wr_t;

typedef struct _test_cmd_rd {
    /* offset 0; */
    uint8_t id;    /* cmd_id; */
    uint8_t cc;    /* complete code */
    uint16_t d0;    /* data 0 */
    uint16_t sn;    /* session number */
    uint16_t chk;    /* 16 bit checksum */
} ctp_tst_rd_t;

#define DIRECTLY_MODE   (0x0)
#define DEDICATE_MODE   (0x1)
#define LEN_CMD_CHK_TX  (10)
#define LEN_CMD_PKG_TX  (16)
#define LEN_RSP_CHK_RX  (8)
#define MAX_BULK_SIZE   (1024)

/* to direct memory access mode */
static uint8_t cmd_2dma_42bd[6] = { /*0x42, 0xbd, */ 0x28, 0x35, 0xc1, 0x00, 0x35, 0xae };
struct ts_fw_infos {
    uint16_t chsc6x_cfg_version;  //customer read 
    uint16_t chsc6x_boot_version; //customer read 
    uint16_t chsc6x_vendor_id;    //customer read 
    uint16_t chsc6x_project_id;   //customer read 
    uint16_t chsc6x_chip_id;      //customer read 
    uint16_t chsc6x_chip_type;    //customer read 
    uint16_t chsc6x_rpt_lcd_x;    //customer read must after chsc6x_get_chip_info
    uint16_t chsc6x_rpt_lcd_y;    //customer read must after chsc6x_get_chip_info
    uint16_t chsc6x_max_pt_num;   //customer read must after chsc6x_get_chip_info
};


/* FUNC In your systerm init process,Must call this interface function to detec if the TP IC is Chipsemi corp'. 
 PARM pfw_infos: to get top 5 fw info in struct ts_fw_infos.
 PARM update_ret_flag: point value=1 update succeed; point value=0 update failed, If opend CHSC6X_AUTO_UPGRADE macro.
 RETURN 1:is chsc chip, 0:is not chsc chip
*/
int chsc6x_tp_dect(struct ts_fw_infos *pfw_infos, uint8_t *update_ret_flag);

/* FUNC: get fw info in struct ts_fw_infos you can call this func anytime.
 PARM pfw_infos: can get all fw infos in struct ts_fw_infos, after call this interface.
 RETURN NULL
*/
void chsc6x_get_chip_info(struct ts_fw_infos *infos);
int chsc6x_read_bytes_u8addr(uint8_t id, uint8_t addr, uint8_t *rxbuf, uint16_t lenth);
int chsc6x_write_bytes_u16addr(uint8_t id, uint16_t addr, uint8_t *rxbuf, uint16_t lenth);
int chsc6x_read_bytes_u16addr(uint8_t id, uint16_t adr, uint8_t *rxbuf, uint16_t lenth);
int chsc6x_read_data_u16addr(uint8_t id, uint16_t adr, uint8_t *rxbuf, uint16_t lenth);
int chsc6x_bulk_down_check(uint8_t *pbuf, uint16_t addr, uint16_t len);
int chsc6x_set_dd_mode(void);
void chsc6x_get_normal_chip_id(uint8_t *id);
void chsc6x_get_boot_chip_id(uint8_t *id);

#endif

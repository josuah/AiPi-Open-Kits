#include "bflb_mtimer.h"
#include "bflb_i2c.h"
#include "bflb_cam.h"
#include "bl616_glb.h"
#include "bflb_l1c.h"
#include "bflb_dbi.h"
#include "bflb_gpio.h"
#include "bflb_uart.h"

#include <FreeRTOS.h>
#include <task.h>
#include <event_groups.h>
#include <queue.h>

#include "board.h"
#include "lcd.h"
#include "ring_buffer.h"
#include "LoraBoard.h"


#if (0)
#define LORA_BOARD_DBG(a, ...) printf("[LORA_BOARD dbg]:" a, ##__VA_ARGS__)
#else
#define LORA_BOARD_DBG(a, ...)
#endif

#if (1)
#define LORA_BOARD_INFO(a, ...) printf("[LORA_BOARD info]:" a, ##__VA_ARGS__)
#else
#define LORA_BOARD_INFO(a, ...)
#endif

#define LORA_BOARD_ERR(a, ...)  printf("[LORA_BOARD err]:" a, ##__VA_ARGS__)






static Ring_Buffer_Type uart_rx_rb;
static uint8_t uart_rx_buffer[256];

static struct bflb_device_s* uart1;
static struct bflb_device_s* uart0;

TaskHandle_t LoraBoard_task_hd;

LoraSettings lora_settings = { 0 };

static void uart1_isr(int irq, void* arg)
{
    uint32_t intstatus = bflb_uart_get_intstatus(uart1);
    uint8_t u8_data;

    if (intstatus & UART_INTSTS_RX_FIFO) {
        while (bflb_uart_rxavailable(uart1)) {
            u8_data = bflb_uart_getchar(uart1);
            Ring_Buffer_Write_Byte(&uart_rx_rb, u8_data);
            // bflb_uart_putchar(uart1, u8_data);
            // LORA_BOARD_DBG("%c", u8_data);
        }
        /* notify to task */
        // BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
        // xTaskNotifyIndexedFromISR(LoraBoard_task_hd, UART_ISR_NOTIFY_INDEX, 1, eSetValueWithOverwrite, &pxHigherPriorityTaskWoken);
        // if (pxHigherPriorityTaskWoken) {
        //     portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
        // }
    }
    if (intstatus & UART_INTSTS_RTO) {

        while (bflb_uart_rxavailable(uart1)) {
            u8_data = bflb_uart_getchar(uart1);
            Ring_Buffer_Write_Byte(&uart_rx_rb, u8_data);
            // bflb_uart_putchar(uart1, u8_data);
            // LORA_BOARD_DBG("%c", u8_data);
        }
        bflb_uart_int_clear(uart1, UART_INTCLR_RTO);
        /* notify to task */
        BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
        xTaskNotifyIndexedFromISR(LoraBoard_task_hd, UART_ISR_NOTIFY_INDEX, 1, eSetValueWithOverwrite, &pxHigherPriorityTaskWoken);
        if (pxHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
        }
    }
}
#if 0
static void LoraBoard_task(void* pvParameters)
{
    static uint16_t cnt = 0;

    int ret;
    char str_buff_total[128], str_buff[64];
    uint32_t len;


    lcd_draw_str_ascii16(10, 50, LCD_COLOR_RGB(0x80, 0x80, 0xff), LCD_COLOR_RGB(0, 0, 0), (void*)"wait Ra08 rece...", 100);
    // lcd_draw_str_ascii16(10, 70, LCD_COLOR_RGB(0xff, 0xff, 0xff), LCD_COLOR_RGB(0, 0, 0), (void *)"lcd init ok...", 100);
    // lcd_draw_str_ascii16(10, 90, LCD_COLOR_RGB(0, 0xff, 0), LCD_COLOR_RGB(0, 0, 0), (void *)"wait disp frame src...", 100);

    // vTaskDelay(2000);
    while (1) {
        // vTaskDelay(1000);
        int mjpeg_status = ulTaskNotifyTakeIndexed(UART_ISR_NOTIFY_INDEX, pdTRUE, portMAX_DELAY);
        len = Ring_Buffer_Get_Length(&uart_rx_rb);
        memset(str_buff, 0, sizeof(str_buff));
        Ring_Buffer_Read(&uart_rx_rb, str_buff, len);
        LORA_BOARD_DBG("Ring_Buffer_Get_Length:%d\r\n", len);
        LORA_BOARD_DBG("str_buff:%s\r\n", str_buff);

        if (len < 10 || len > 50) {
            continue;
        }
        if (str_buff[0] == 'i' && str_buff[1] == 'd') {
            if (str_buff[3] == '1') {
                lcd_draw_str_ascii16(10, 50, LCD_COLOR_RGB(0x80, 0x80, 0xff), LCD_COLOR_RGB(0, 0, 0), (void*)str_buff, 100);
            }
            if (str_buff[3] == '2') {
                lcd_draw_str_ascii16(10, 70, LCD_COLOR_RGB(0xff, 0xff, 0xff), LCD_COLOR_RGB(0, 0, 0), (void*)str_buff, 100);
            }
            if (str_buff[3] == '3') {
                lcd_draw_str_ascii16(10, 90, LCD_COLOR_RGB(0, 0xff, 0), LCD_COLOR_RGB(0, 0, 0), (void*)str_buff, 100);
            }
            if (str_buff[3] == '4') {
                lcd_draw_str_ascii16(10, 110, LCD_COLOR_RGB(0x80, 0x80, 0xff), LCD_COLOR_RGB(0, 0, 0), (void*)str_buff, 100);
            }
            if (str_buff[3] == '5') {
                lcd_draw_str_ascii16(10, 130, LCD_COLOR_RGB(0xff, 0xff, 0xff), LCD_COLOR_RGB(0, 0, 0), (void*)str_buff, 100);
            }
            if (str_buff[3] == '6') {
                lcd_draw_str_ascii16(10, 150, LCD_COLOR_RGB(0, 0xff, 0), LCD_COLOR_RGB(0, 0, 0), (void*)str_buff, 100);
            }
        }
    }

}
#else

#include <string.h>
#include "ui.h"
static void LoraBoard_task(void* pvParameters)
{
    static uint16_t ms_cnt = 0;
    int ret;
    char str_buff_total[128], str_buff[64], temp_buf[16], humi_buf[16];
    uint32_t len;

    while (1) {
        // vTaskDelay(1000);
        int mjpeg_status = ulTaskNotifyTakeIndexed(UART_ISR_NOTIFY_INDEX, pdTRUE, portMAX_DELAY);
        if (mjpeg_status == 1) {

            len = Ring_Buffer_Get_Length(&uart_rx_rb);
            memset(str_buff, 0, sizeof(str_buff));
            Ring_Buffer_Read(&uart_rx_rb, str_buff, len);
            LORA_BOARD_DBG("Ring_Buffer_Get_Length:%d\r\n", len);
            LORA_BOARD_DBG("str_buff:%s\r\n", str_buff);

            if (len < 10 || len > 50) {
                continue;
            }
            if (str_buff[0] == 'i' && str_buff[1] == 'd') {
                memset(temp_buf, 0, sizeof(temp_buf));
                memcpy(temp_buf, &str_buff[5], 10);
                memset(humi_buf, 0, sizeof(humi_buf));
                memcpy(humi_buf, &str_buff[17], 7);
                memset(str_buff_total, 0, sizeof(str_buff_total));
                snprintf(str_buff_total, 128, "      %s\n      %s", temp_buf, humi_buf);
                if (str_buff[3] == '1') {
                    lv_label_set_text(ui_LabelSensor1, str_buff_total);
                }
                if (str_buff[3] == '2') {
                    lv_label_set_text(ui_LabelSensor2, str_buff_total);
                }
                if (str_buff[3] == '3') {
                    lv_label_set_text(ui_LabelSensor3, str_buff_total);
                }
                if (str_buff[3] == '4') {
                    lv_label_set_text(ui_LabelSensor4, str_buff_total);
                }
                if (str_buff[3] == '5') {
                    lv_label_set_text(ui_LabelSensor5, str_buff_total);
                }
                if (str_buff[3] == '6') {
                    lv_label_set_text(ui_LabelSensor6, str_buff_total);
                }
            }
        }
        else if (mjpeg_status == 2) {
            // uart0 = bflb_device_get_by_name("uart0");
            ms_cnt = 0;
            memset(str_buff_total, 0, sizeof(str_buff_total));
            snprintf(str_buff_total, 128, "AT+SETRF=%d,%d,%d,%d,%d\r\n", lora_settings.lora_freq, lora_settings.lora_power, lora_settings.lora_bw, lora_settings.lora_sf, lora_settings.lora_crc);
            bflb_uart_put(uart1, str_buff_total, strlen(str_buff_total));
            LORA_BOARD_DBG("sendCmd:%s\r\n", str_buff_total);
            // bflb_uart_put(uart0, str_buff_total, strlen(str_buff_total));
            while (ms_cnt <= 200)
            {
                ms_cnt++;
                vTaskDelay(10);
                len = Ring_Buffer_Get_Length(&uart_rx_rb);
                if (len >= 2) {
                    memset(str_buff, 0, sizeof(str_buff));
                    Ring_Buffer_Read(&uart_rx_rb, str_buff, len);
                    LORA_BOARD_DBG("str_buff:%s\r\n", str_buff);
                    if (strstr(str_buff, "OK") != NULL) {
                        lv_label_set_text(ui_Label2, "Lora Setting  Status : OK");
                        break;
                    }
                }


            }
            if (ms_cnt >= 200) {
                lv_label_set_text(ui_Label2, "Lora Setting  Status : Timeout");
            }
        }
    }
}
#endif
static void uart1_init()
{
    struct bflb_device_s* gpio;

    gpio = bflb_device_get_by_name("gpio");
    bflb_gpio_uart_init(gpio, GPIO_PIN_18, GPIO_UART_FUNC_UART1_TX);
    bflb_gpio_uart_init(gpio, GPIO_PIN_19, GPIO_UART_FUNC_UART1_RX);

    struct bflb_uart_config_s cfg;
    cfg.baudrate = 115200;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;

    uart1 = bflb_device_get_by_name("uart1");

    bflb_uart_init(uart1, &cfg);
    bflb_uart_rxint_mask(uart1, false);
    bflb_irq_attach(uart1->irq_num, uart1_isr, NULL);
    bflb_irq_enable(uart1->irq_num);
}
int LoraBoard_task_init(void)
{
    // dbi_hd = bflb_device_get_by_name("dbi");
    // gpio = bflb_device_get_by_name("gpio");

    uart1_init();
    Ring_Buffer_Init(&uart_rx_rb, uart_rx_buffer, sizeof(uart_rx_buffer), NULL, NULL);

    /* lcd init */
    // lcd_init();
    // lcd_set_dir(1, 0);
    // lcd_clear(LCD_COLOR_RGB(0, 0, 0));
    // printf("lcd init\r\n");

    // bflb_mtimer_delay_ms(1000);
    /* creat task */
    xTaskCreate(LoraBoard_task, (char*)"LoraBoard_task", 1024*3, NULL, 20, &LoraBoard_task_hd);

    return 0;
}
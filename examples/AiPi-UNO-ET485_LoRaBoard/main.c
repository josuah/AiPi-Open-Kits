/****************************************************************************
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
 ****************************************************************************/

 /****************************************************************************
  * Included Files
  ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <queue.h>
#include <semphr.h>

#include <lwip/tcpip.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

#include "bl_fw_api.h"
#include "wifi_mgmr_ext.h"
#include "wifi_mgmr.h"

#include "bflb_irq.h"
#include "bflb_uart.h"

#include "bl616_glb.h"
#include "rfparam_adapter.h"

#include "board.h"
#include "shell.h"

#define DBG_TAG "MAIN"
#include "log.h"
  //kiwi
#include "lv_conf.h"
#include "lvgl.h"

#include "lv_port_disp.h"
#include "lv_port_indev.h"


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define WIFI_STACK_SIZE  (1536)
#define TASK_PRIORITY_FW (16)

 /****************************************************************************
  * Private Types
  ****************************************************************************/

  /****************************************************************************
   * Private Data
   ****************************************************************************/

static struct bflb_device_s* uart0;
struct bflb_device_s* gpio;


static TaskHandle_t wifi_fw_task;

static wifi_conf_t conf = {
    .country_code = "CN",
};

extern void shell_init_with_task(struct bflb_device_s* shell);

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
extern void udp_multicast_task(void* arg);
extern int jpeg_lcd_task_init(void);
extern void audio_server_task_create(void);
/****************************************************************************
 * Functions
 ****************************************************************************/

int wifi_start_firmware_task(void)
{
    LOG_I("Starting wifi ...\r\n");

    /* enable wifi clock */

    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_IP_WIFI_PHY | GLB_AHB_CLOCK_IP_WIFI_MAC_PHY | GLB_AHB_CLOCK_IP_WIFI_PLATFORM);
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_WIFI);

    if (0 != rfparam_init(0, NULL, 0)) {
        LOG_I("PHY RF init failed!\r\n");
        return 0;
    }

    LOG_I("PHY RF init success!\r\n");

    /* Enable wifi irq */

    extern void interrupt0_handler(void);
    bflb_irq_attach(WIFI_IRQn, (irq_callback)interrupt0_handler, NULL);
    bflb_irq_enable(WIFI_IRQn);

    xTaskCreate(wifi_main, (char*)"fw", WIFI_STACK_SIZE, NULL, TASK_PRIORITY_FW, &wifi_fw_task);

    return 0;
}

void wifi_event_handler(uint32_t code)
{
    switch (code) {
        case CODE_WIFI_ON_INIT_DONE: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_INIT_DONE\r\n", __func__);
            wifi_mgmr_init(&conf);
        } break;
        case CODE_WIFI_ON_MGMR_DONE: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_MGMR_DONE\r\n", __func__);
        } break;
        case CODE_WIFI_ON_SCAN_DONE: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_SCAN_DONE\r\n", __func__);
            wifi_mgmr_sta_scanlist();
        } break;
        case CODE_WIFI_ON_CONNECTED: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_CONNECTED\r\n", __func__);
            void mm_sec_keydump();
            mm_sec_keydump();
#if (defined(CONFIG_SOLUTION_FUNC_HIBOOSTER) && CONFIG_SOLUTION_FUNC_HIBOOSTER && !CONFIG_SOLUTION_FUNC_MJPEG_APP)
            // bl_cam_frame_init();
            // create_http_server_task();
            // xTaskCreate(udp_multicast_task, "udp_multicast_task", 2048, NULL, 16, NULL);
#endif
        } break;
        case CODE_WIFI_ON_GOT_IP: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_GOT_IP\r\n", __func__);

        } break;
        case CODE_WIFI_ON_DISCONNECT: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_DISCONNECT\r\n", __func__);
        } break;
        case CODE_WIFI_ON_AP_STARTED: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_AP_STARTED\r\n", __func__);

            // audio_server_task_create();
        } break;
        case CODE_WIFI_ON_AP_STOPPED: {
            LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_AP_STOPPED\r\n", __func__);
        } break;
        case CODE_WIFI_ON_AP_STA_ADD: {
            LOG_I("[APP] [EVT] [AP] [ADD] %lld\r\n", xTaskGetTickCount());
            // jpeg_lcd_task_init();
            // audio_init();
        } break;
        case CODE_WIFI_ON_AP_STA_DEL: {
            LOG_I("[APP] [EVT] [AP] [DEL] %lld\r\n", xTaskGetTickCount());
        } break;
        default: {
            LOG_I("[APP] [EVT] Unknown code %u \r\n", code);
        }
    }
}


static void fps_printf_task(void* pvParameters)
{
    while (1) {
        for (uint32_t i = 0; i < 5; i++) {
            printf("FPS ");
            printf("\r\n");
            vTaskDelay(1000);
        }
#if (configGENERATE_RUN_TIME_STATS)
        static char info_buffer[1024];
        vTaskGetRunTimeStats(info_buffer);
        printf("\r\n<taskname>   <runtime>   <cpu load>\r\n%s\r\n", info_buffer);
#endif
    }
}


uint8_t wifi_connect(char* ssid, char* passwd)
{
    int ret = 255;
    // struct fhost_vif_ip_addr_cfg ip_cfg = {0};

    if (NULL==ssid || 0==strlen(ssid)) {
        return 1;
    }

    if (wifi_mgmr_sta_state_get() == 1) {
        wifi_sta_disconnect();
    }
    if (wifi_sta_connect(ssid, passwd, NULL, NULL, 0, 0, 0, 1)) {
        return 4;
    }

    //等待连接成功
    // sta_ConnectStatus = 0;
    // for(int i=0;i<10*30;i++){
    // 	vTaskDelay(100);
    // 	switch(sta_ConnectStatus){
    // 		case CODE_WIFI_ON_CONNECTED:	//连接成功(表示wifi sta状态的时候表示同时获取IP(DHCP)成功，或者使用静态IP)
    // 			return 0;
    // 		case CODE_WIFI_ON_DISCONNECT:	//连接失败（超过了重连次数还没有连接成功的状态）
    // 			wifi_sta_disconnect();
    // 			return 4;
    // 		case CODE_WIFI_ON_GOT_IP:	
    // 			return 0;
    // 		default:
    // 			//等待连接成功
    // 			break;
    // 	}
    // }

    return 14; //连接超时
}

void lv_log_print_g_cb(const char* buf)
{
    printf("[LVGL] %s", buf);
}

static TaskHandle_t lvgl_TaskHandle;
#define LVGL_STACK_SIZE 1024*2
#define LVGL_TASK_PRIORITY 15

void lvgl_task(void* param)
{
    gpio = bflb_device_get_by_name("gpio");
    // bflb_gpio_init(gpio, GPIO_PIN_18, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);
    // bflb_gpio_set(gpio, GPIO_PIN_18);
    while (1)
    {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

extern int LoraBoard_task_init(void);
int main(void)
{
    /* core voltage cfg */
    // HBN_Set_Ldo11_Rt_Vout(12);
    // HBN_Set_Ldo11_Soc_Vout(12);

    /* board init */
    board_init();

    // bflb_irq_set_nlbits(4);
    // bflb_irq_set_priority(37, 3, 0);
    // bflb_irq_set_priority(WIFI_IRQn, 1, 0);

    /* shell init */
    uart0 = bflb_device_get_by_name("uart0");
    shell_init_with_task(uart0);
    printf("\r\nShell ready\r\n");

    /* network init */
    tcpip_init(NULL, NULL);
    /* wifi init */
    wifi_start_firmware_task();

    /* all queue init */

    /****************** solution audio ******************/

    /* lvgl init */
    lv_log_register_print_cb(lv_log_print_g_cb);
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    ui_init();
    xTaskCreate(lvgl_task, (char*)"lvgl", LVGL_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, &lvgl_TaskHandle);
    LoraBoard_task_init();

    vTaskStartScheduler();

    while (1) {
    }
}

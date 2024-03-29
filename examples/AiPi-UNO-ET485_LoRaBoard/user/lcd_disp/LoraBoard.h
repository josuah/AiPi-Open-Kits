#ifndef LORABOARD_H_
#define LORABOARD_H_

#define UART_ISR_NOTIFY_INDEX 0


typedef struct{
    uint32_t lora_freq;
    uint8_t lora_power;
    uint8_t lora_sf;
    uint8_t lora_bw;
    uint8_t lora_crc;
}LoraSettings;
extern LoraSettings lora_settings;


#endif
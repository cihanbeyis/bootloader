#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>

// Flash bellek işlemleri için değişkenler
extern uint16_t current_flash_address;
extern uint8_t flash_page_buffer[SPM_PAGESIZE];
extern uint16_t flash_page_offset;

// HEX formatı işleme fonksiyonları
uint8_t hex_to_bin(char c);
uint8_t hex_pair_to_byte(char high, char low);

// Flash bellek işlemleri
void flash_page_write(void);
void process_hex_data(uint8_t *data, uint16_t length);

// Uygulama kontrol ve atlama fonksiyonları
uint8_t is_application_valid(void);
void jump_to_application(void);

#endif // HELPER_H

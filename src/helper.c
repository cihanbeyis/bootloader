#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "helper.h"
#include "uart.h"
#include "spi.h"
#include "w5100.h"

// Flash bellek işlemleri için değişkenler
uint16_t current_flash_address = 0;
uint8_t flash_page_buffer[SPM_PAGESIZE];
uint16_t flash_page_offset = 0;

// HEX formatı işleme fonksiyonları
uint8_t hex_to_bin(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

uint8_t hex_pair_to_byte(char high, char low) {
    return (hex_to_bin(high) << 4) | hex_to_bin(low);
}

// Flash belleğe yazma fonksiyonu
void flash_page_write(void) {
    uint16_t i;
    uint8_t sreg;
    
    // Kesmeleri devre dışı bırak
    sreg = SREG;
    cli();
    
    // Sayfayı temizle
    boot_page_erase(current_flash_address);
    boot_spm_busy_wait();
    
    // Sayfayı doldur
    for (i = 0; i < SPM_PAGESIZE; i += 2) {
        uint16_t word = flash_page_buffer[i] | (flash_page_buffer[i + 1] << 8);
        boot_page_fill(current_flash_address + i, word);
    }
    
    // Sayfayı yaz
    boot_page_write(current_flash_address);
    boot_spm_busy_wait();
    
    // Yazma modunu etkinleştir
    boot_rww_enable();
    
    // Kesmeleri geri aç
    SREG = sreg;
    
    // Bir sonraki sayfaya geç
    current_flash_address += SPM_PAGESIZE;
    flash_page_offset = 0;
}

// HEX dosyası veri işleme fonksiyonu
void process_hex_data(uint8_t *data, uint16_t length) {
    uint16_t i = 0;
    
    while (i < length) {
        char line_buffer[50];
        uint8_t line_length = 0;
        
        // Satır başlangıcını bul
        while (i < length && data[i] != ':') i++;
        
        if (i >= length) break;
        i++; // ':' karakterini atla
        
        // Satırı oku
        while (i < length && data[i] != '\r' && data[i] != '\n' && line_length < sizeof(line_buffer) - 1) {
            line_buffer[line_length++] = data[i++];
        }
        line_buffer[line_length] = '\0';
        
        // Satır sonu karakterlerini atla
        while (i < length && (data[i] == '\r' || data[i] == '\n')) i++;
        
        // Satırı işle
        if (line_length >= 10) { // Minimum geçerli HEX satırı uzunluğu
            uint8_t byte_count = hex_pair_to_byte(line_buffer[0], line_buffer[1]);
            uint16_t address = (hex_pair_to_byte(line_buffer[2], line_buffer[3]) << 8) | 
                                hex_pair_to_byte(line_buffer[4], line_buffer[5]);
            uint8_t record_type = hex_pair_to_byte(line_buffer[6], line_buffer[7]);
            
            // Veri kaydı
            if (record_type == 0 && byte_count > 0) {
                uint16_t absolute_address = address;
                
                // Flash sayfasını ayarla
                if (current_flash_address == 0 || 
                    (absolute_address / SPM_PAGESIZE) != (current_flash_address / SPM_PAGESIZE)) {
                    
                    // Önceki sayfayı yaz (eğer varsa)
                    if (flash_page_offset > 0) {
                        flash_page_write();
                    }
                    
                    // Yeni sayfayı ayarla
                    current_flash_address = (absolute_address / SPM_PAGESIZE) * SPM_PAGESIZE;
                    flash_page_offset = absolute_address % SPM_PAGESIZE;
                }
                
                // Veriyi flash page buffer'a kopyala
                for (uint8_t j = 0; j < byte_count; j++) {
                    uint8_t data_byte = hex_pair_to_byte(line_buffer[8 + j*2], line_buffer[9 + j*2]);
                    flash_page_buffer[flash_page_offset++] = data_byte;
                    
                    // Sayfa doldu mu?
                    if (flash_page_offset >= SPM_PAGESIZE) {
                        flash_page_write();
                    }
                }
            }
            // EOF kaydı
            else if (record_type == 1) {
                // Son sayfayı yaz (eğer varsa)
                if (flash_page_offset > 0) {
                    flash_page_write();
                }
                break;
            }
        }
    }
}

// Uygulama geçerliliğini kontrol eden fonksiyon
uint8_t is_application_valid(void) {
    // Flash belleğin ilk 32 byte'ını kontrol et
    uint8_t all_ff = 1; // Başlangıçta tümü 0xFF olarak kabul et
    
    for (uint16_t i = 0; i < 32; i++) {
        uint8_t byte = pgm_read_byte(i);
        if (byte != 0xFF) {
            all_ff = 0; // En az bir byte 0xFF değil
            break;
        }
    }
    
    // Eğer tüm byte'lar 0xFF ise, flash boş demektir
    if (all_ff) {
        return 0; // Geçerli uygulama yok
    }
    
    // Reset vektörünü kontrol et (ilk 2 byte)
    uint16_t reset_vector = pgm_read_word(0);
    
    // RJMP veya JMP talimatı olup olmadığını kontrol et
    if ((reset_vector & 0xF000) == 0xC000) { // RJMP talimatı
        return 1; // Geçerli uygulama var
    }
    
    // Eğer buraya kadar geldiyse, uygulama geçerli olmayabilir
    // Ancak en azından boş değil, bu yüzden bir şey var
    return 1;
}

// Uygulamaya atlama fonksiyonu
void jump_to_application(void) {
    // Tüm çevre birimlerini devre dışı bırak
    cli();                    // Tüm kesmeleri devre dışı bırak
    UCSR0B = 0;               // UART'ı kapat
    SPCR = 0;                 // SPI'yı kapat
    wdt_disable();            // Watchdog'u kapat
    
    // Uygulamaya atla
    asm volatile(
        "ldi r30, 0\n\t"      // Z register'ının düşük byte'ını 0 yap
        "ldi r31, 0\n\t"      // Z register'ının yüksek byte'ını 0 yap
        "ijmp\n\t"            // Z register'ındaki adrese atla (0x0000)
    );
}

void test_network(void) {
    uint8_t gateway_ip[4] = {192, 168, 1, 1}; // Ağ geçidi IP adresi
    
    // Debug mesajını gönder
    uart_puts_p(PSTR("Pinging gateway "));
    uart_putip(gateway_ip);
    uart_puts_p(PSTR("...\r\n"));
    
    if (w5100_ping(gateway_ip)) {
        debug_message(PSTR("Ping successful!"));
    } else {
        debug_message(PSTR("Ping failed!"));
    }
}


// main.c - W5100 TFTP Bootloader ana dosyası
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "uart.h"
#include "spi.h"
#include "w5100.h"
#include "network.h"
#include "tftp.h"
#include "settings.h"
#include "eeprom.h"
#include "monitor.h"
#include "helper.h"
#include "timer.h"  // Timer fonksiyonlarını içeren header dosyası eklendi

// Güncelleme bayrağı değerleri
#define UPDATE_REQUIRED     0xAA
#define UPDATE_COMPLETED    0x00  // 0x00 yerine 0xFF kullanıyorum (boş EEPROM değeri)

// Ana program
int main(void) {
    // Watchdog'u devre dışı bırak (eğer önceki sıfırlamadan kaldıysa)
    wdt_disable();
    
    // Sistem başlatma
    uart_init();
    
    // Başlangıç mesajı
    uart_puts("\r\nW5100 TFTP Bootloader v1.0.0\r\n");
    
    // SPI başlatma
    spi_init();
    uart_puts("SPI initialized\r\n");
    
    // EEPROM başlatma
    eeprom_init();
    uart_puts("EEPROM initialized\r\n");
    
    // Timer başlatma
    timer_init();
    uart_puts("Timer initialized\r\n");
    
    // Global kesmeleri etkinleştir
    sei();
    uart_puts("Interrupts enabled\r\n");
    
    // Güncelleme bayrağını kontrol et
    uint8_t update_flag = eeprom_read_update_flag();
    uart_puts("Update flag value: 0x");
    print_hex(update_flag);
    uart_puts("\r\n");
    
    // Ağı her durumda başlat ve durumunu göster
    uart_puts("Initializing network...\r\n");
    if (net_init()) {
        uart_puts("Network initialized successfully\r\n");
        monitor_print_all();  // Ağ durumunu her zaman göster
    } else {
        uart_puts("Network initialization failed\r\n");
    }
    
    // Güncelleme bayrağı ayarlanmamışsa, doğrudan uygulamaya atla
    if (update_flag != UPDATE_REQUIRED) {
        uart_puts("Update flag not set, jumping to application...\r\n");
        
        // Uygulama geçerli mi kontrol et
        if (is_application_valid()) {
            uart_puts("Valid application found, jumping...\r\n");
            _delay_ms(100);  // UART mesajının gönderilmesini bekle
            jump_to_application();
        } else {
            uart_puts("No valid application found, staying in bootloader\r\n");
            // Bootloader'da kal
        }
    } else {
        uart_puts("Update flag is set (0xAA), starting firmware update...\r\n");
        
        // TFTP sunucu IP adresini oku
        eeprom_read_tftp_server_ip(tftp_server_ip);
        
        // Ağ başlatma kontrolü
        if (!net_init()) {
            uart_puts("Network initialization failed\r\n");
            uart_puts("Staying in bootloader due to network failure\r\n");
            
            // Bootloader'da kal, sonsuz döngüye gir
            while(1) {
                _delay_ms(1000);
                uart_puts("N");  // Network hatası için 'N' yazdır
            }
        }
        
        // TFTP ile firmware indir - YORUM SATIRINDA KALSIN
        uart_puts("Starting TFTP download of firmware.hex...\r\n");
        uint8_t download_success = tftp_download_file("firmware.hex", process_hex_data);
        
        if (download_success) {
           uart_puts("Firmware successfully loaded!\r\n");
           // Güncelleme bayrağını sıfırla
           eeprom_write_update_flag(UPDATE_COMPLETED);
           uart_puts("Update flag cleared (0x00)\r\n");
        } else {
           uart_puts("Failed to load firmware!\r\n");
           // Güncelleme bayrağını değiştirmeden bırak, böylece bir sonraki başlatmada tekrar denenebilir
           uart_puts("Update flag remains set for next boot\r\n");
        }
        
        // Test amaçlı olarak, TFTP indirme işlemi olmadan bayrağı temizle
        // eeprom_write_update_flag(UPDATE_COMPLETED);
        // uart_puts("Update flag cleared for testing (0x00)\r\n");
        
        // uart_puts("Jumping to application...\r\n");
        
        // Uygulama geçerli mi kontrol et
        if (is_application_valid()) {
            uart_puts("Valid application found, jumping...\r\n");
            _delay_ms(100);
            jump_to_application();
        } else {
            uart_puts("No valid application found, staying in bootloader\r\n");
            // Bootloader'da kal
        }
    }
    
    // Eğer buraya kadar geldiyse, bootloader'da kalıyoruz
    uart_puts("Staying in bootloader\r\n");
    
    // Sonsuz döngüde bekle, sürekli resetlemeyi önle
    while(1) {
        // Bootloader'da kal
        _delay_ms(1000);
        uart_puts(".");
    }
    
    return 0;  // Buraya asla ulaşılmaz
}

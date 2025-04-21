// monitor.c
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "w5100.h"
#include "network.h"
#include "eeprom.h"
#include "monitor.h"

// LED işlemleri için yeni bir fonksiyon adı kullanın (eğer gerekliyse)
// Örneğin led_blink yerine monitor_led_blink kullanabilirsiniz
// Ya da bu fonksiyonu tamamen kaldırabilirsiniz

void monitor_init(void) {
    // Monitor başlatma kodları
    uart_puts("Monitor initialized\r\n");
}

void monitor_print_all(void) {
    uint8_t version;
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t subnet[4];
    uint8_t gateway[4];
    uint8_t tftp_server[4];
    
    // EEPROM'dan yapılandırma bilgilerini oku
    eeprom_read_mac(mac);
    eeprom_read_device_ip(ip);
    eeprom_read_subnet_mask(subnet);
    eeprom_read_gateway(gateway);
    eeprom_read_tftp_server_ip(tftp_server);
    
    // W5100 versiyon bilgisini oku
    version = w5100_read(0x0039);  // Version register
    
    uart_puts("\r\nNetwork Configuration:\r\n");
    
    uart_puts("W5100 Version: 0x");
    print_hex(version);
    uart_puts("\r\n");
    
    uart_puts("MAC: ");
    for (uint8_t i = 0; i < 6; i++) {
        print_hex(mac[i]);
        if (i < 5) uart_putc(':');
    }
    uart_puts("\r\n");
    
    uart_puts("IP: ");
    uart_putip(ip);
    uart_puts("\r\n");
    
    uart_puts("Subnet: ");
    uart_putip(subnet);
    uart_puts("\r\n");
    
    uart_puts("Gateway: ");
    uart_putip(gateway);
    uart_puts("\r\n");
    
    uart_puts("TFTP Server: ");
    uart_putip(tftp_server);
    uart_puts("\r\n");
    
    // Socket 0 durumunu kontrol et
    uint8_t status = w5100_read(W5100_S0_SR);
    uart_puts("Socket 0 Status: 0x");
    print_hex(status);
    uart_puts("\r\n");
}

void monitor_task(void) {
    // Periyodik görevler (gerekirse)
}

// Burada main() fonksiyonu vardı, kaldırıldı

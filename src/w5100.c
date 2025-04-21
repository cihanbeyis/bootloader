/*
 * W5100 TFTP Bootloader - W5100 Kontrol
 * Bu dosya W5100 Ethernet kontrolcüsü ile iletişim için gerekli fonksiyonları içerir
 */

 #include <avr/io.h>           // AVR I/O tanımlamaları
 #include <stdio.h>  // sprintf için
 #include <stdlib.h> // itoa için
 #include <util/delay.h> // _delay_ms için
 #include <string.h>
        // Gecikme fonksiyonları
 #include "w5100.h"            // W5100 tanımlamaları
 #include "spi.h"              // SPI iletişim fonksiyonları
 #include "uart.h"             // Yardımcı fonksiyonlar
 
 void w5100_cs_low(void) {
    W5100_PORT &= ~(1 << W5100_CS);
}

void w5100_cs_high(void) {
    W5100_PORT |= (1 << W5100_CS);
}

 // W5100 başlatma fonksiyonu
// W5100 başlatma fonksiyonu
void w5100_init(void) {
    // W5100 reset - Donanımsal sıfırlama
    // Reset pin Arduino Uno'da pin 9'a bağlı olduğunu varsayıyoruz (PORTB1)
    DDRB |= _BV(PB1);        // Reset pini çıkış olarak ayarla
    PORTB &= ~_BV(PB1);      // Reset pini LOW yaparak reset sinyali gönder
    _delay_ms(10);           // Reset sinyali için bekle
    PORTB |= _BV(PB1);       // Reset pinini HIGH yaparak normal çalışmaya geç
    _delay_ms(100);          // W5100'ün başlaması için yeterli süre bekle
    
    // SPI CS pinini ayarla
    W5100_DDR |= (1 << W5100_CS);  // CS pinini çıkış olarak ayarla
    w5100_cs_high();               // CS başlangıçta yüksek (deaktif)
    
    // Mode Register - Software Reset (Yazılımsal sıfırlama)
    w5100_cs_low();                // CS aktif (düşük)
    w5100_write(W5100_MR, 0x80);   // 0x80 değeri software reset bitini aktifleştirir
    w5100_cs_high();               // CS deaktif (yüksek)
    _delay_ms(10);                 // Reset tamamlanması için bekle
    
    // RX/TX Memory size ayarı - Socket 0 için tüm bellek
    w5100_cs_low();                // CS aktif (düşük)
    w5100_write(W5100_RMSR, 0x03); // Socket 0 için 8KB RX bellek (0x03 = 8KB)
    w5100_write(W5100_TMSR, 0x03); // Socket 0 için 8KB TX bellek (0x03 = 8KB)
    w5100_cs_high();               // CS deaktif (yüksek)
    
}

 // W5100 kapatma fonksiyonu
 void w5100_close(void) {
     // Tüm socketleri kapat - Temiz bir kapatma için
     w5100_socket_close(0);  // Soket 0'ı kapat (sadece bu soketi kullanıyoruz)
 }
 
 // Register okuma fonksiyonu - Belirtilen adresteki değeri okur
 uint8_t w5100_read(uint16_t addr) {
     uint8_t data;
     
     spi_enable();  // SPI iletişimini başlat (CS pini LOW)
     
     spi_transfer(0x0F);         // Okuma komutu (0x0F)
     spi_transfer(addr >> 8);    // Adres high byte
     spi_transfer(addr & 0xFF);  // Adres low byte
     data = spi_transfer(0);     // Veri okuma (0 göndererek clock üretiyoruz)
     
     spi_disable();  // SPI iletişimini sonlandır (CS pini HIGH)
     
     return data;  // Okunan veriyi döndür
 }

 uint16_t w5100_read_word(uint16_t addr) {
    uint16_t data;
    data = (uint16_t)w5100_read(addr) << 8;
    data |= (uint16_t)w5100_read(addr + 1);
    return data;
}

 void w5100_read_data(uint8_t socket, uint16_t offset, uint8_t *buf, uint16_t len) {
    uint16_t addr = W5100_RX_BASE + (socket * 0x0800) + (offset & 0x07FF);
    w5100_read_addr(addr, buf, len);
}

 
 // Register yazma fonksiyonu - Belirtilen adrese değer yazar
 void w5100_write(uint16_t addr, uint8_t data) {
     spi_enable();  // SPI iletişimini başlat (CS pini LOW)
     
     spi_transfer(0xF0);         // Yazma komutu (0xF0)
     spi_transfer(addr >> 8);    // Adres high byte
     spi_transfer(addr & 0xFF);  // Adres low byte
     spi_transfer(data);         // Veri yazma
     
     spi_disable();  // SPI iletişimini sonlandır (CS pini HIGH)
 }
 
 // Çoklu register okuma fonksiyonu - Belirtilen adresten itibaren birden fazla byte okur
 void w5100_read_addr(uint16_t addr, uint8_t *buf, uint16_t len) {
     for (uint16_t i = 0; i < len; i++) {
         buf[i] = w5100_read(addr + i);  // Her byte için okuma işlemi
     }
 }
 
 // Çoklu register yazma fonksiyonu - Belirtilen adrese birden fazla byte yazar
 void w5100_write_addr(uint16_t addr, const uint8_t *buf, uint16_t len) {
     for (uint16_t i = 0; i < len; i++) {
         w5100_write(addr + i, buf[i]);  // Her byte için yazma işlemi
     }
 }
 
 // Socket açma fonksiyonu - Belirtilen modda ve portta bir soket açar
 uint8_t w5100_socket_open(uint8_t socket, uint8_t mode, uint16_t port, uint8_t flag) {
    // Soket durumunu kontrol et
    if (w5100_read(W5100_S0_SR + socket * 0x0100) != W5100_SOCK_CLOSED) {
        w5100_socket_close(socket);
    }
    
    // Port ayarla
    w5100_write(W5100_S0_PORT + socket * 0x0100, (port >> 8) & 0xFF);
    w5100_write(W5100_S0_PORT + socket * 0x0100 + 1, port & 0xFF);
    
    // Modu ayarla ve OPEN komutunu gönder
    w5100_write(W5100_S0_MR + socket * 0x0100, mode);
    w5100_write(W5100_S0_CR + socket * 0x0100, W5100_CMD_OPEN);
    
    // Komut tamamlanana kadar bekle
    while (w5100_read(W5100_S0_CR + socket * 0x0100));
    
    // Soket durumunu kontrol et
    return (w5100_read(W5100_S0_SR + socket * 0x0100) == W5100_SOCK_UDP) ? 1 : 0;
}
// Socket kapatma fonksiyonu
 void w5100_socket_close(uint8_t sock) {
     // Socket kapat komutu gönder
     w5100_write(W5100_S0_CR + sock * 0x0100, W5100_CMD_CLOSE);
     
     // Komutun tamamlanmasını bekle (CR register 0 olana kadar)
     while (w5100_read(W5100_S0_CR + sock * 0x0100));
 }
 
 // Socket veri alma fonksiyonu - Soketten gelen veriyi okur
 uint16_t w5100_socket_receive(uint8_t sock, uint8_t *buf, uint16_t bufsize) {
     // Alınan veri uzunluğunu kontrol et (RX Received Size Register)
     uint16_t len = (w5100_read(W5100_S0_RX_RSR + sock * 0x0100) << 8) |
                    w5100_read(W5100_S0_RX_RSR + sock * 0x0100 + 1);
     
     if (len == 0) {
         return 0;  // Alınacak veri yok
     }
     
     // Alınacak veri uzunluğunu tampon boyutuyla sınırla
     if (len > bufsize) {
         len = bufsize;
     }
     
     // RX read pointer al - Okuma yapılacak bellek adresi
     uint16_t ptr = (w5100_read(W5100_S0_RX_RD + sock * 0x0100) << 8) |
                    w5100_read(W5100_S0_RX_RD + sock * 0x0100 + 1);
     
     // Veriyi oku - RX belleğinden veri kopyala
     uint16_t offset = ptr & 0x07FF;  // 2KB bellek sınırı için maske (8KB için 0x1FFF olmalı)
     uint16_t base = W5100_RX_BASE + sock * 0x0800;  // Soket RX bellek başlangıcı
     
     for (uint16_t i = 0; i < len; i++) {
         buf[i] = w5100_read(base + ((offset + i) & 0x07FF));  // Dairesel tampon için maske
     }
     
     // RX read pointer güncelle - Okunan veri kadar ilerlet
     ptr += len;
     w5100_write(W5100_S0_RX_RD + sock * 0x0100, ptr >> 8);       // High byte
     w5100_write(W5100_S0_RX_RD + sock * 0x0100 + 1, ptr & 0xFF); // Low byte
     
     // Receive komutu gönder - Okuma işleminin tamamlandığını bildir
     w5100_write(W5100_S0_CR + sock * 0x0100, W5100_CMD_RECV);
     
     // Komutun tamamlanmasını bekle
     while (w5100_read(W5100_S0_CR + sock * 0x0100));
     
     return len;  // Okunan veri uzunluğunu döndür
 }
 
 // Socket veri gönderme fonksiyonu - Soketten veri gönderir
 void w5100_socket_send(uint8_t sock, uint8_t *buf, uint16_t bufsize) {
     // TX write pointer al - Yazma yapılacak bellek adresi
     uint16_t ptr = (w5100_read(W5100_S0_TX_WR + sock * 0x0100) << 8) |
                    w5100_read(W5100_S0_TX_WR + sock * 0x0100 + 1);
     
     // Veriyi yaz - TX belleğine veri kopyala
     uint16_t offset = ptr & 0x07FF;  // 2KB bellek sınırı için maske (8KB için 0x1FFF olmalı)
     uint16_t base = W5100_TX_BASE + sock * 0x0800;  // Soket TX bellek başlangıcı
     
     for (uint16_t i = 0; i < bufsize; i++) {
         w5100_write(base + ((offset + i) & 0x07FF), buf[i]);  // Dairesel tampon için maske
     }
     
     // TX write pointer güncelle - Yazılan veri kadar ilerlet
     ptr += bufsize;
     w5100_write(W5100_S0_TX_WR + sock * 0x0100, ptr >> 8);       // High byte
     w5100_write(W5100_S0_TX_WR + sock * 0x0100 + 1, ptr & 0xFF); // Low byte
     
     // Send komutu gönder - Gönderme işlemini başlat
     w5100_write(W5100_S0_CR + sock * 0x0100, W5100_CMD_SEND);
     
     // Komutun tamamlanmasını bekle
     while (w5100_read(W5100_S0_CR + sock * 0x0100));
 }

 void w5100_debug_status(void) {
    debug_message(PSTR("W5100 Durum:\r\n"));
    
    // Genel durum
    debug_message(PSTR("Mode Register: 0x"));
    char hex_str[3];
    sprintf(hex_str, "%02X", w5100_read(W5100_MR));
    uart_puts(hex_str);
    uart_puts("\r\n");
    
    // Socket 0 durumu
    debug_message(PSTR("Socket 0 Status: 0x"));
    sprintf(hex_str, "%02X", w5100_read(W5100_S0_SR));
    uart_puts(hex_str);
    uart_puts("\r\n");
    
    // Socket 0 interrupt durumu
    debug_message(PSTR("Socket 0 IR: 0x"));
    sprintf(hex_str, "%02X", w5100_read(W5100_S0_IR));
    uart_puts(hex_str);
    uart_puts("\r\n");
    
    // RX/TX buffer durumu
    uint16_t rx_size = w5100_read_word(W5100_S0_RX_RSR);
    uint16_t tx_free = w5100_read_word(W5100_S0_TX_FSR);
    
    debug_message(PSTR("RX Size: "));
    char size_str[6];
    itoa(rx_size, size_str, 10);
    uart_puts(size_str);
    uart_puts("\r\n");
    
    debug_message(PSTR("TX Free Size: "));
    itoa(tx_free, size_str, 10);
    uart_puts(size_str);
    uart_puts("\r\n");
}

uint8_t w5100_ping(uint8_t *ip_addr) {
    uint8_t sock = 0; // Ping için soket 0 kullanılacak
    uint8_t result = 0;
    uint8_t ping_data[] = {0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01};
    uint16_t checksum = 0;
    uint16_t ptr;
    uint16_t timeout;
    
    // Soket kapatılıp temizlenir
    w5100_socket_close(sock);
    _delay_ms(100);
    
    // Soket ICMP modunda açılır (IP RAW modu)
    w5100_write(W5100_S0_MR, W5100_MODE_IPRAW); // IP RAW modu (ICMP için)
    w5100_write(W5100_S0_CR, W5100_CMD_OPEN); // OPEN komutu
    _delay_ms(10);
    
    // Hedef IP adresi ayarlanır
    w5100_write(W5100_S0_DIPR0, ip_addr[0]);
    w5100_write(W5100_S0_DIPR0 + 1, ip_addr[1]);
    w5100_write(W5100_S0_DIPR0 + 2, ip_addr[2]);
    w5100_write(W5100_S0_DIPR0 + 3, ip_addr[3]);
    
    // Basit checksum hesaplama
    for (uint8_t i = 0; i < sizeof(ping_data); i += 2) {
        uint16_t word = ((uint16_t)ping_data[i] << 8);
        if (i + 1 < sizeof(ping_data)) {
            word |= ping_data[i + 1];
        }
        checksum += word;
    }
    
    // 16-bit checksum hesaplama
    checksum = (checksum >> 8) + (checksum & 0xFF);
    checksum = ~checksum & 0xFFFF;
    ping_data[2] = (checksum >> 8) & 0xFF;
    ping_data[3] = checksum & 0xFF;
    
    // TX buffer'a veri yazılır
    ptr = (w5100_read(W5100_S0_TX_WR0) << 8) | w5100_read(W5100_S0_TX_WR0 + 1);
    
    // TX buffer'a veri yazılır
    for (uint8_t i = 0; i < sizeof(ping_data); i++) {
        w5100_write(W5100_TX_BASE + (ptr & 0x07FF), ping_data[i]);
        ptr++;
    }
    
    // TX pointer güncellenir
    w5100_write(W5100_S0_TX_WR0, (ptr >> 8) & 0xFF);
    w5100_write(W5100_S0_TX_WR0 + 1, ptr & 0xFF);
    
    // SEND komutu gönderilir
    w5100_write(W5100_S0_CR, W5100_CMD_SEND);
    
    // Yanıt bekle (timeout ile)
    timeout = 0;
    while (timeout < 1000) {
        if (w5100_read(W5100_S0_IR) & 0x04) { // RECV flag kontrolü
            result = 1;
            break;
        }
        _delay_ms(1);
        timeout++;
    }
    
    // Soketi kapat
    w5100_socket_close(sock);
    
    return result;
}

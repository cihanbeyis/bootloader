/*
 * W5100 TFTP Bootloader - Ağ İşlemleri
 * Bu dosya ağ işlemleri için gerekli fonksiyonları içerir
 */

 #include <avr/io.h>         // AVR I/O tanımlamaları
 #include <util/delay.h> 
 #include <stdlib.h>    // Gecikme fonksiyonları
 #include "network.h"            // Ağ tanımlamaları
 #include "w5100.h"          // W5100 fonksiyonları
 #include "uart.h"  
 #include "eeprom.h"         // Yardımcı fonksiyonlar
 
 
 // Ağ başlatma fonksiyonu - Ethernet donanımını ve ağ ayarlarını başlatır
 uint8_t net_init(void) {
    uint8_t ip_addr[4];
    uint8_t subnet[4];
    uint8_t gateway[4];
    uint8_t mac[6];
    
    // EEPROM'dan ağ ayarlarını oku
    eeprom_read_device_ip(ip_addr);
    eeprom_read_subnet_mask(subnet);
    eeprom_read_gateway(gateway);
    eeprom_read_mac(mac);
    
    // W5100 başlatma
    w5100_init();
    
    // Ağ ayarlarını yapılandır
    
    // Gateway ayarla
    w5100_write(W5100_GAR0 + 0, gateway[0]);
    w5100_write(W5100_GAR0 + 1, gateway[1]);
    w5100_write(W5100_GAR0 + 2, gateway[2]);
    w5100_write(W5100_GAR0 + 3, gateway[3]);
    
    // Subnet mask ayarla
    w5100_write(W5100_SUBR0 + 0, subnet[0]);
    w5100_write(W5100_SUBR0 + 1, subnet[1]);
    w5100_write(W5100_SUBR0 + 2, subnet[2]);
    w5100_write(W5100_SUBR0 + 3, subnet[3]);
    
    // MAC adresi ayarla
    w5100_write(W5100_SHAR0 + 0, mac[0]);
    w5100_write(W5100_SHAR0 + 1, mac[1]);
    w5100_write(W5100_SHAR0 + 2, mac[2]);
    w5100_write(W5100_SHAR0 + 3, mac[3]);
    w5100_write(W5100_SHAR0 + 4, mac[4]);
    w5100_write(W5100_SHAR0 + 5, mac[5]);
    
    // IP adresi ayarla
    w5100_write(W5100_SIPR0 + 0, ip_addr[0]);
    w5100_write(W5100_SIPR0 + 1, ip_addr[1]);
    w5100_write(W5100_SIPR0 + 2, ip_addr[2]);
    w5100_write(W5100_SIPR0 + 3, ip_addr[3]);
    
    // TX/RX bellek boyutlarını ayarla
    w5100_write(W5100_RMSR, 0x55);  // Her soket için 2KB RX belleği
    w5100_write(W5100_TMSR, 0x55);  // Her soket için 2KB TX belleği
    
    // Socket 0'ı UDP modunda aç
    uint16_t random_port = 10000 + (TCNT1 % 50000);

    // Socket 0'ı UDP modunda aç
    w5100_socket_open(0, W5100_MODE_UDP, random_port, 0);
    
    debug_message(PSTR("Socket opened with local port: "));
    char port_str[6];
    itoa(random_port, port_str, 10);
    uart_puts(port_str);
    uart_puts("\r\n");

    // Başlatma başarılı
    debug_message(PSTR("Network initialized"));
    
    return 1;
}

// Paket alma fonksiyonu
uint16_t net_receive_packet(uint8_t *buf, uint16_t bufsize) {
    uint16_t len = 0;
    uint8_t sock_status;
    
    // Socket durumunu kontrol et
    sock_status = w5100_read(W5100_S0_SR);
    
    if (sock_status != W5100_SOCK_UDP) {
        // Socket UDP modunda değilse, yeniden aç
        w5100_socket_close(0);
        w5100_socket_open(0, W5100_MODE_UDP, TFTP_PORT, 0);
        return 0;
    }
    
    // Gelen veri var mı kontrol et
    len = (w5100_read(W5100_S0_RX_RSR) << 8) | w5100_read(W5100_S0_RX_RSR + 1);
    
    if (len > 0) {
        // Veri varsa, oku
        if (len > bufsize) {
            len = bufsize;  // Buffer boyutunu aşma
        }
        
        // Paketi oku
        w5100_read_addr(W5100_RX_BASE, buf, len);
        
        // Okuma işaretçisini güncelle
        uint16_t ptr = (w5100_read(W5100_S0_RX_RD) << 8) | w5100_read(W5100_S0_RX_RD + 1);
        ptr += len;
        w5100_write(W5100_S0_RX_RD, (ptr >> 8) & 0xFF);
        w5100_write(W5100_S0_RX_RD + 1, ptr & 0xFF);
        
        // RECV komutu ile okuma işlemini onayla
        w5100_write(W5100_S0_CR, W5100_CMD_RECV);
        
        return len;
    }
    
    return 0;  // Veri yok
}

// Paket gönderme fonksiyonu
void net_send_packet(uint8_t *buf, uint16_t bufsize) {
    // Gönderim için yeterli alan var mı kontrol et
    uint16_t free_size = (w5100_read(W5100_S0_TX_FSR) << 8) | w5100_read(W5100_S0_TX_FSR + 1);
    
    if (free_size < bufsize) {
        return;  // Yeterli alan yok
    }
    
    // Paketi yaz
    uint16_t ptr = (w5100_read(W5100_S0_TX_WR) << 8) | w5100_read(W5100_S0_TX_WR + 1);
    w5100_write_addr(W5100_TX_BASE + (ptr & 0x1FFF), buf, bufsize);
    
    // Yazma işaretçisini güncelle
    ptr += bufsize;
    w5100_write(W5100_S0_TX_WR, (ptr >> 8) & 0xFF);
    w5100_write(W5100_S0_TX_WR + 1, ptr & 0xFF);
    
    // SEND komutu ile gönderimi başlat
    w5100_write(W5100_S0_CR, W5100_CMD_SEND);
    
    // Gönderim tamamlanana kadar bekle
    while (w5100_read(W5100_S0_CR));
}

// UDP paketi gönderme fonksiyonu - Belirli bir IP ve porta
uint8_t net_send_udp_packet(uint8_t *dest_ip, uint16_t dest_port, uint8_t *buf, uint16_t bufsize) {
    uint8_t status, ir;
    uint16_t timeout_counter = 0;
    
    // Socket durumunu kontrol et
    status = w5100_read(W5100_S0_SR);
    if (status != W5100_SOCK_UDP) {
        uart_puts_p(PSTR("[ERROR] Socket is not in UDP mode: 0x"));
        uart_puthex(status);
        uart_puts_p(PSTR("\r\n"));
        return 0;
    }
    
    // Hedef IP adresini ayarla
    w5100_write(W5100_S0_DIPR + 0, dest_ip[0]);
    w5100_write(W5100_S0_DIPR + 1, dest_ip[1]);
    w5100_write(W5100_S0_DIPR + 2, dest_ip[2]);
    w5100_write(W5100_S0_DIPR + 3, dest_ip[3]);
    
    // Hedef portu ayarla
    w5100_write(W5100_S0_DPORT, (dest_port >> 8) & 0xFF);
    w5100_write(W5100_S0_DPORT + 1, dest_port & 0xFF);
    
    // Gönderim için yeterli alan var mı kontrol et
    uint16_t free_size = (w5100_read(W5100_S0_TX_FSR) << 8) | w5100_read(W5100_S0_TX_FSR + 1);
    
    if (free_size < bufsize) {
        uart_puts_p(PSTR("[ERROR] Not enough TX buffer: "));
        
        // free_size ve bufsize değerlerini yazdır
        uart_printf("%u < %u\r\n", free_size, bufsize);
        return 0;
    }
    
    // Interrupt register'ını temizle
    w5100_write(W5100_S0_IR, 0xFF);
    
    // Paketi yaz
    uint16_t ptr = (w5100_read(W5100_S0_TX_WR) << 8) | w5100_read(W5100_S0_TX_WR + 1);
    
    // Tampon belleğe veri yaz
    for (uint16_t i = 0; i < bufsize; i++) {
        w5100_write(W5100_TX_BASE + ((ptr + i) & 0x1FFF), buf[i]);
    }
    
    // Yazma işaretçisini güncelle
    ptr += bufsize;
    w5100_write(W5100_S0_TX_WR, (ptr >> 8) & 0xFF);
    w5100_write(W5100_S0_TX_WR + 1, ptr & 0xFF);
    
    // SEND komutu ile gönderimi başlat
    w5100_write(W5100_S0_CR, W5100_CMD_SEND);
    
    // Gönderim tamamlanana kadar bekle (timeout ile)
    timeout_counter = 0;
    while (w5100_read(W5100_S0_CR)) {
        _delay_ms(1);
        timeout_counter++;
        if (timeout_counter > 1000) { // 1 saniye timeout
            uart_puts_p(PSTR("[ERROR] Send command timeout\r\n"));
            return 0;
        }
    }
    
    // SEND_OK bayrağını kontrol et (0x10 = SEND_OK bit)
    timeout_counter = 0;
    while (1) {
        ir = w5100_read(W5100_S0_IR);
        if (ir & 0x10) { // 0x10 = SEND_OK bit (W5100 için standart)
            // SEND_OK bayrağını temizle
            w5100_write(W5100_S0_IR, 0x10);
            break;
        }
        
        _delay_ms(1);
        timeout_counter++;
        if (timeout_counter > 1000) { // 1 saniye timeout
            uart_puts_p(PSTR("[ERROR] SEND_OK timeout\r\n"));
            return 0;
        }
    }
    
    uart_puts_p(PSTR("[SUCCESS] Packet sent successfully\r\n"));
    return 1;  // Başarılı
}

// UDP paket alma fonksiyonu
uint8_t network_udp_receive(uint16_t port, uint8_t *buf, uint16_t *len, uint16_t *src_port) {
    uint8_t src_ip[4];
    uint8_t header[8];
    uint16_t received_port;
    uint16_t data_len;
    
    // Socket durumunu kontrol et
    uint8_t sock_status = w5100_read(W5100_S0_SR);
    
    if (sock_status != W5100_SOCK_UDP) {
        // Socket UDP modunda değilse, yeniden aç
        w5100_socket_close(0);
        w5100_socket_open(0, W5100_MODE_UDP, port, 0);
        return 0;
    }
    
    // Gelen veri var mı kontrol et
    uint16_t rx_size = (w5100_read(W5100_S0_RX_RSR) << 8) | w5100_read(W5100_S0_RX_RSR + 1);
    
    if (rx_size > 0) {
        // Paket başlığını oku (8 byte)
        uint16_t rx_rd = (w5100_read(W5100_S0_RX_RD) << 8) | w5100_read(W5100_S0_RX_RD + 1);
        w5100_read_data(0, rx_rd, header, 8);
        
        // Kaynak IP adresini ve portu al
        src_ip[0] = header[0];
        src_ip[1] = header[1];
        src_ip[2] = header[2];
        src_ip[3] = header[3];
        received_port = (header[4] << 8) | header[5];
        
        // Veri uzunluğunu al
        data_len = (header[6] << 8) | header[7];
        
        // Veri boyutu kontrolü
        if (data_len > *len) {
            data_len = *len;  // Buffer boyutunu aşma
        }
        
        // Veriyi oku
        w5100_read_data(0, rx_rd + 8, buf, data_len);
        
        // RX pointer'ı güncelle
        rx_rd += data_len + 8;
        w5100_write(W5100_S0_RX_RD, (rx_rd >> 8) & 0xFF);
        w5100_write(W5100_S0_RX_RD + 1, rx_rd & 0xFF);
        
        // RECV komutu gönder
        w5100_write(W5100_S0_CR, W5100_CMD_RECV);
        
        // Alınan veri uzunluğunu ve kaynak portu döndür
        *len = data_len;
        *src_port = received_port;
        
        return 1;  // Paket başarıyla alındı
    }
    
    return 0;  // Paket yok
}

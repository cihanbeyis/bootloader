// tftp.c - W5100 TFTP Bootloader için TFTP protokolü uygulaması

#include <stdint.h>
#include <string.h>
#include <stdlib.h>  // itoa için
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include "tftp.h"
#include "w5100.h"
#include "uart.h"
#include "network.h"
#include "timer.h"

// TFTP değişkenleri
uint8_t tftp_packet_buffer[TFTP_PACKET_MAX_SIZE]; // extern olarak tanımlandı, static değil
uint16_t tftp_block_number; // extern olarak tanımlandı, static değil
uint16_t tftp_server_port; // extern olarak tanımlandı, static değil
uint8_t tftp_server_ip[4];  // extern olarak tanımlandı

// Sadece bu dosyada kullanılan static değişkenler
static uint8_t tftp_retries;
static uint32_t tftp_timeout_counter;
static uint16_t tftp_client_port;

// Rastgele bir istemci portu ayarla (1024-65535 arası)
static void tftp_set_random_client_port(void) {
    // Basit bir rastgele port oluşturma
    uint16_t random_value = TCNT1; // Timer değerini kullan
    tftp_client_port = 1024 + (random_value % (65535 - 1024));
}

// TFTP protokolünü başlat
void tftp_init(void) {
    tftp_block_number = 1;  // RFC 1350'ye göre ilk veri paketi blok 1'dir
    tftp_retries = 0;
    tftp_timeout_counter = 0;
    tftp_set_random_client_port();
    
    // Soketi kapat
    w5100_socket_close(0);
    
    // Soketi UDP modunda aç
    w5100_socket_open(0, W5100_MODE_UDP, tftp_client_port, 0);
    
    // Soket durumunu kontrol et
    if (w5100_read(W5100_S0_SR) != W5100_SOCK_UDP) {
        debug_message(PSTR("UDP socket not opened"));
        return;
    }
    
    char port_str[6];
    itoa(tftp_client_port, port_str, 10);
    debug_message(PSTR("TFTP client port: "));
    uart_puts(port_str);
    uart_puts("\r\n");
    
    debug_message(PSTR("TFTP protocol initialized"));
}

// TFTP okuma isteği (RRQ) gönder
uint8_t tftp_send_read_request(const char* filename) {
    uint16_t packet_length = 0;
    uint8_t *p = tftp_packet_buffer;
    uint8_t result;
    
    // Opcode: RRQ (1)
    *p++ = 0;
    *p++ = TFTP_RRQ;
    packet_length += 2;
    
    // Dosya adı
    strcpy((char*)p, filename);
    p += strlen(filename) + 1;  // +1 for null terminator
    packet_length += strlen(filename) + 1;
    
    // Transfer modu ("octet")
    strcpy((char*)p, "octet");
    p += 6;  // "octet" + null terminator
    packet_length += 6;
    
    // Debug için paket içeriğini yazdır
    uart_puts_p(PSTR("RRQ packet sending... "));
    
    // Paket içeriğini hexadecimal olarak yazdır
    uart_puts_p(PSTR("Packet content: "));
    for (uint16_t i = 0; i < packet_length; i++) {
        uart_puthex(tftp_packet_buffer[i]);
        uart_putc(' ');
    }
    uart_puts_p(PSTR("\r\n"));
    
    tftp_server_port = TFTP_SERVER_PORT;
    
    // Socket durumunu kontrol et
    uint8_t socket_status = getSn_SR(TFTP_SOCKET);
    uart_puts_p(PSTR("Socket status before send: 0x"));
    uart_puthex(socket_status);
    uart_puts_p(PSTR("\r\n"));
    
    result = net_send_udp_packet(tftp_server_ip, tftp_server_port,
                                tftp_packet_buffer, packet_length);
    
    // Gönderim sonucu
    uart_puts_p(PSTR("Send result: "));
    uart_puthex(result);
    uart_puts_p(PSTR("\r\n"));
    
    // Socket durumunu tekrar kontrol et
    socket_status = getSn_SR(TFTP_SOCKET);
    uart_puts_p(PSTR("Socket status after send: 0x"));
    uart_puthex(socket_status);
    uart_puts_p(PSTR("\r\n"));
    
    return result;
}

// TFTP onay paketi (ACK) gönder
uint8_t tftp_send_ack(uint16_t block_number) {
    uint8_t *p = tftp_packet_buffer;
    
    // Opcode: ACK (4)
    *p++ = 0;
    *p++ = TFTP_ACK;
    
    // Block number
    *p++ = (block_number >> 8) & 0xFF;
    *p++ = block_number & 0xFF;
    
    debug_message(PSTR("ACK packet sending... "));
    
    return net_send_udp_packet(tftp_server_ip, tftp_server_port,
                              tftp_packet_buffer, 4);
}

uint8_t tftp_send_ack_with_retry(uint16_t block_number, uint8_t max_retries) {
    uint8_t retry_count = 0;
    
    while (retry_count < max_retries) {
        if (tftp_send_ack(block_number)) {
            debug_message(PSTR("ACK sent successfully"));
            return 1;  // Başarılı
        }
        
        debug_message(PSTR("ACK sending failed, retrying..."));
        retry_count++;
        _delay_ms(100);  // Kısa bir bekleme
    }
    
    debug_message(PSTR("ACK sending failed after max retries"));
    return 0;  // Başarısız
}

// TFTP hata paketini işle
void tftp_handle_error(uint8_t *packet, uint16_t packet_length) {
    if (packet_length < 4) {
        debug_message(PSTR("Invalid error packet"));
        return;
    }
    
    uint16_t error_code = (packet[2] << 8) | packet[3];
    
    debug_message(PSTR("TFTP Error: "));
    
    char code_str[6];
    itoa(error_code, code_str, 10);
    uart_puts(code_str);
    uart_puts(" - ");
    
    if (packet_length > 4) {
        uart_puts((char*)&packet[4]);
    }
    
    uart_puts("\r\n");
}

// TFTP veri paketini işle
uint8_t tftp_handle_data(uint8_t *packet, uint16_t packet_length, tftp_data_callback_t callback) {
    if (packet_length < 4) {
        debug_message(PSTR("Invalid data packet"));
        return 0;
    }
    
    uint16_t block_number = (packet[2] << 8) | packet[3];
    uint16_t data_length = packet_length - 4;
    
    // Debug bilgisi
    debug_message(PSTR("Data received, block: "));
    char block_str[6];
    itoa(block_number, block_str, 10);
    uart_puts(block_str);
    uart_puts(", Expected: ");
    char expected_str[6];
    itoa(tftp_block_number, expected_str, 10);
    uart_puts(expected_str);
    uart_puts("\r\n");
    
    // Beklenen blok numarası mı?
    if (block_number != tftp_block_number) {
        debug_message(PSTR("Unexpected block number"));
        
        // Eğer önceki bloğun tekrarıysa veya daha önceki bir bloksa, ACK gönder ve atla
        if (block_number < tftp_block_number) {
            debug_message(PSTR("Resending ACK for previous block"));
            if (!tftp_send_ack_with_retry(block_number, 3)) {
                debug_message(PSTR("ACK resend failed"));
                return 0;
            }
            return 1;  // Başarılı ama yeni veri yok
        } else {
            // Gelecekteki bir blok - bu durumda transfer'i yeniden başlatmak gerekebilir
            debug_message(PSTR("Received future block, transfer might be corrupted"));
            return 0;  // Başarısız
        }
    }
    
    // Veriyi callback ile işle
    if (callback && data_length > 0) {
        callback(&packet[4], data_length);
    }
    
    // ACK gönder (yeniden deneme mekanizması ile)
    if (!tftp_send_ack_with_retry(block_number, 3)) {
        debug_message(PSTR("ACK sending failed after retries"));
        return 0;
    }
    
    // Blok numarasını güncelle
    tftp_block_number++;
    
    // Son paket mi? (512 byte'dan az veri içeriyorsa)
    if (data_length < 512) {
        debug_message(PSTR("Transfer completed"));
        return 2;  // Transfer tamamlandı
    }
    
    return 1;  // Başarılı, devam ediyor
}

// TFTP paketini işle
uint8_t tftp_process_packet(uint8_t *packet, uint16_t packet_length, tftp_data_callback_t callback) {
    if (packet_length < 2) {
        debug_message(PSTR("Invalid TFTP packet"));
        return 0;
    }
    
    uint16_t opcode = (packet[0] << 8) | packet[1];
    
    switch (opcode) {
        case TFTP_DATA:
            return tftp_handle_data(packet, packet_length, callback);
            
        case TFTP_ERROR:
            tftp_handle_error(packet, packet_length);
            return 0;
            
        default:
            debug_message(PSTR("Unknown opcode: "));
            char opcode_str[6];
            itoa(opcode, opcode_str, 10);
            uart_puts(opcode_str);
            uart_puts("\r\n");
            return 0;
    }
}

// TFTP dosya indirme - tftp.h'deki tanımla uyumlu hale getirildi
uint8_t tftp_download_file(const char *filename, tftp_data_callback_t callback) {
    uint8_t retries = 0;
    uint8_t result;
    
    // tftp_server_ip ve tftp_server_port zaten global değişkenlerde tanımlı olmalı
    
    while (retries < 3) {  // 3 kez deneme
        // Blok numarasını sıfırla
        tftp_block_number = 1;
        
        // İndirmeyi başlat
        debug_message(PSTR("Starting TFTP download..."));
        result = tftp_start_download(filename, callback);
        
        if (result == 2) {  // Transfer tamamlandı
            debug_message(PSTR("Download completed successfully"));
            return 1;
        } else if (result == 1) {  // Devam ediyor, ancak tamamlanmadı
            debug_message(PSTR("Download in progress but interrupted"));
        } else {  // Başarısız
            debug_message(PSTR("Download failed"));
        }
        
        debug_message(PSTR("Retrying download..."));
        retries++;
        _delay_ms(1000);  // 1 saniye bekle
    }
    
    debug_message(PSTR("Download failed after max retries"));
    return 0;  // Başarısız
}

// TFTP indirme işlemini başlat - tftp.h'deki tanımla uyumlu hale getirildi
uint8_t tftp_start_download(const char *filename, tftp_data_callback_t callback) {
    uint8_t *p = tftp_packet_buffer;
    uint16_t len = 0;
    
    // Opcode: RRQ (1)
    *p++ = 0;
    *p++ = TFTP_RRQ;
    len += 2;
    
    // Filename
    while (*filename) {
        *p++ = *filename++;
        len++;
    }
    *p++ = 0;  // Null-terminate
    len++;
    
    // Mode ("octet")
    const char *mode = "octet";
    while (*mode) {
        *p++ = *mode++;
        len++;
    }
    *p++ = 0;  // Null-terminate
    len++;
    
    debug_message(PSTR("Sending RRQ..."));
    
    // network.h'deki net_send_udp_packet fonksiyonu ile uyumlu hale getirildi
    if (!net_send_udp_packet(tftp_server_ip, tftp_server_port, tftp_packet_buffer, len)) {
        debug_message(PSTR("RRQ sending failed"));
        return 0;
    }
    
    // Timeout değerlerini ayarla
    uint16_t timeout_ms = 5000;  // 5 saniye
    uint32_t start_time = timer_get();  // millis() yerine timer_get() kullanıldı
    uint16_t src_port = 0;  // Kaynak port değişkeni
    
    // Veri paketlerini bekle ve işle
    while (1) {
        // Timeout kontrolü
        if (timer_get() - start_time > timeout_ms) {  // millis() yerine timer_get() kullanıldı
            debug_message(PSTR("Timeout waiting for data"));
            return 0;
        }
        
        // network.h'deki network_udp_receive fonksiyonu ile paket alımı
        uint16_t packet_length = TFTP_PACKET_MAX_SIZE;  // Maksimum paket boyutu
        uint8_t packet_buffer[TFTP_PACKET_MAX_SIZE];    // Geçici paket tamponu
        
        // UDP paketi geldi mi kontrol et
        if (network_udp_receive(tftp_client_port, packet_buffer, &packet_length, &src_port)) {
            // İlk pakette sunucu portunu kaydet (TID - Transfer ID)
            if (src_port != tftp_server_port && tftp_block_number == 1) {
                tftp_server_port = src_port;
                debug_message(PSTR("Server TID updated: "));
                char port_str[6];
                itoa(tftp_server_port, port_str, 10);
                uart_puts(port_str);
                uart_puts("\r\n");
            }
            
            // Paketi işle
            uint8_t result = tftp_process_packet(packet_buffer, packet_length, callback);
            
            if (result == 2) {  // Transfer tamamlandı
                return 2;
            } else if (result == 0) {  // Hata
                return 0;
            }
            
            // Yeni paket için timeout'u sıfırla
            start_time = timer_get();  // millis() yerine timer_get() kullanıldı
        }
        
        // Kısa bir bekleme ile CPU kullanımını azalt
        _delay_ms(1);
    }
    
    return 0;  // Buraya asla ulaşılmamalı
}

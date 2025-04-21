// tftp.h - W5100 TFTP Bootloader için TFTP protokolü başlık dosyası
#ifndef TFTP_H
#define TFTP_H

#include <stdint.h>

// TFTP yapılandırma sabitleri
#define TFTP_SERVER_PORT  69        // TFTP sunucu portu
#define TFTP_PACKET_MAX_SIZE 516    // 512 byte veri + 4 byte header
#define TFTP_MAX_RETRIES  5         // Maksimum yeniden deneme sayısı
#define TFTP_TIMEOUT_MS   5000      // Timeout süresi (ms)

// TFTP protokol sabitleri (opcode değerleri)
#define TFTP_RRQ   1  // Okuma isteği
#define TFTP_WRQ   2  // Yazma isteği
#define TFTP_DATA  3  // Veri paketi
#define TFTP_ACK   4  // Onay paketi
#define TFTP_ERROR 5  // Hata paketi

// TFTP için kullanılacak soket numarası
#define TFTP_SOCKET 0  // Socket 0 kullanılacak

// Global değişkenler
extern uint8_t tftp_server_ip[4];  // TFTP sunucu IP adresi
extern uint16_t tftp_block_number;
extern uint16_t tftp_server_port;
extern uint8_t tftp_packet_buffer[TFTP_PACKET_MAX_SIZE];

// TFTP veri callback fonksiyon tipi
typedef void (*tftp_data_callback_t)(uint8_t *data, uint16_t length);

// TFTP fonksiyonları
void tftp_init(void);
uint8_t tftp_send_read_request(const char* filename);
uint8_t tftp_send_ack(uint16_t block_number);
uint8_t tftp_send_ack_with_retry(uint16_t block_number, uint8_t max_retries);
uint8_t tftp_download_file(const char *filename, tftp_data_callback_t callback);
uint8_t tftp_handle_data(uint8_t *packet, uint16_t packet_length, tftp_data_callback_t callback);
void tftp_handle_error(uint8_t *packet, uint16_t packet_length);
uint8_t tftp_process_packet(uint8_t *packet, uint16_t packet_length, tftp_data_callback_t callback);
uint8_t tftp_start_download(const char *filename, tftp_data_callback_t callback);

#endif // TFTP_H

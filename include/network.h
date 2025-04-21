// network.h
#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

// MAC adresi - Ethernet cihazının benzersiz donanım adresi
// Bu adres genellikle rastgele seçilebilir, ancak ağda benzersiz olmalıdır
#define MAC_ADDR    {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}

// Statik IP yapılandırması - İstenilen sabit IP adresi ve ağ ayarları
#define IP_ADDR     {192, 168, 1, 207}  // Arduino'nun statik IP adresi
#define SUBNET_MASK {255, 255, 255, 0}  // Alt ağ maskesi (24-bit ağ)
#define GATEWAY     {192, 168, 1, 1}    // Ağ geçidi (router) adresi

// TFTP yapılandırması - TFTP protokolü için port ayarları
#define TFTP_PORT   69      // TFTP standart portu (RFC 1350)
#define DATA_PORT   12345   // Veri transferi için kullanılacak dinamik port

// Fonksiyon prototipleri

// Ağ başlatma fonksiyonu - Ethernet donanımını ve ağ ayarlarını başlatır
uint8_t net_init(void);

// Paket alma fonksiyonu - Ağdan gelen paketi alır
uint16_t net_receive_packet(uint8_t *buf, uint16_t bufsize);

// Paket gönderme fonksiyonu - Ağa paket gönderir
void net_send_packet(uint8_t *buf, uint16_t len);
uint8_t net_send_udp_packet(uint8_t *dest_ip, uint16_t dest_port, uint8_t *buf, uint16_t bufsize);
// network.h dosyasına eklenecek
// UDP paket alma fonksiyonu
uint8_t network_udp_receive(uint16_t port, uint8_t *buf, uint16_t *len, uint16_t *src_port);

#endif

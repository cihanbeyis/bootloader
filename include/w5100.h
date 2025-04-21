/*
 * W5100 TFTP Bootloader - W5100 Kontrol
 * Bu dosya W5100 Ethernet kontrolcüsü ile iletişim için gerekli tanımlamaları içerir
 */

 #ifndef _W5100_H_
 #define _W5100_H_
 
 #include <stdint.h>
 
 // W5100 register adresleri
 #define W5100_MR       0x0000    // Mode Register - Cihaz çalışma modunu kontrol eder
 #define W5100_GAR0     0x0001    // Gateway Address - Ağ geçidi adresinin başlangıcı
 #define W5100_SUBR0    0x0005    // Subnet Mask Address - Alt ağ maskesinin başlangıcı
 #define W5100_SHAR0    0x0009    // Source Hardware Address (MAC) - MAC adresinin başlangıcı
 #define W5100_SIPR0    0x000F    // Source IP Address - IP adresinin başlangıcı
 #define W5100_RMSR     0x001A    // RX Memory Size - Alıcı bellek boyutu ayarı
 #define W5100_TMSR     0x001B    // TX Memory Size - Verici bellek boyutu ayarı
 
 // Socket register base adresleri - Her soket için ayrı kontrol registerleri
 #define W5100_S0_BASE  0x0400    // Socket 0 base address - Soket 0 için başlangıç adresi
 #define W5100_S0_MR    0x0400    // Socket 0 Mode Register - Soket 0 mod ayarı
 #define W5100_S0_CR    0x0401    // Socket 0 Command Register - Soket 0 komut registeri
 #define W5100_S0_IR    0x0402    // Socket 0 Interrupt Register - Soket 0 kesme registeri
 #define W5100_S0_SR    0x0403    // Socket 0 Status Register - Soket 0 durum registeri
 #define W5100_S0_PORT  0x0404    // Socket 0 Source Port - Soket 0 kaynak port
 
 // Socket 0 için hedef adres ve port registerleri
 #define W5100_S0_DIPR0 0x040C    // Socket 0 Destination IP Address - Hedef IP adresi başlangıcı
 #define W5100_S0_DPORT0 0x0410   // Socket 0 Destination Port - Hedef port başlangıcı
 

 #ifndef W5100_S0_DIPR1
#define W5100_S0_DIPR1 (W5100_S0_DIPR0 + 1)
#endif

#ifndef W5100_S0_DIPR2
#define W5100_S0_DIPR2 (W5100_S0_DIPR0 + 2)
#endif

#ifndef W5100_S0_DIPR3
#define W5100_S0_DIPR3 (W5100_S0_DIPR0 + 3)
#endif

#ifndef W5100_S0_TX_WR1
#define W5100_S0_TX_WR1 (W5100_S0_TX_WR0 + 1)
#endif

 // Socket 0 için TX/RX register'ları
 #define W5100_S0_TX_FSR0   0x0420    // Socket 0 TX Free Size Register - TX boş alan boyutu
 #define W5100_S0_TX_RD0    0x0422    // Socket 0 TX Read Pointer - TX okuma işaretçisi
 #define W5100_S0_TX_WR0    0x0424    // Socket 0 TX Write Pointer - TX yazma işaretçisi
 #define W5100_S0_RX_RSR0   0x0426    // Socket 0 RX Received Size Register - RX alınan veri boyutu
 #define W5100_S0_RX_RD0    0x0428    // Socket 0 RX Read Pointer - RX okuma işaretçisi
 
 // Kısaltmalar için tanımlar
 #define W5100_S0_DIPR      W5100_S0_DIPR0    // Socket 0 Destination IP Address
 #define W5100_S0_DPORT     W5100_S0_DPORT0   // Socket 0 Destination Port
 #define W5100_S0_TX_FSR    W5100_S0_TX_FSR0  // Socket 0 TX Free Size Register
 #define W5100_S0_TX_RD     W5100_S0_TX_RD0   // Socket 0 TX Read Pointer
 #define W5100_S0_TX_WR     W5100_S0_TX_WR0   // Socket 0 TX Write Pointer
 #define W5100_S0_RX_RSR    W5100_S0_RX_RSR0  // Socket 0 RX Received Size Register
 #define W5100_S0_RX_RD     W5100_S0_RX_RD0   // Socket 0 RX Read Pointer
 
 // Socket TX/RX memory offset - Veri tamponu adresleri
 #define W5100_TX_BASE  0x4000    // TX buffer base address - Gönderme tamponu başlangıç adresi
 #define W5100_RX_BASE  0x6000    // RX buffer base address - Alma tamponu başlangıç adresi
 
 // Socket modları - Soketin çalışma modları
 #define W5100_MODE_TCP   0x01    // TCP modu
 #define W5100_MODE_UDP   0x02    // UDP modu (TFTP için bu kullanılacak)
 #define W5100_MODE_IPRAW 0x03    // IP RAW modu
 #define W5100_MODE_MACRAW 0x04   // MAC RAW modu
 #define W5100_MODE_PPPOE 0x05    // PPPoE modu
 
 // Socket komutları - Soket işlemleri için komutlar
 #define W5100_CMD_OPEN      0x01 // Soket açma komutu
 #define W5100_CMD_LISTEN    0x02 // Dinleme komutu (TCP için)
 #define W5100_CMD_CONNECT   0x04 // Bağlantı kurma komutu (TCP için)
 #define W5100_CMD_DISCON    0x08 // Bağlantı kesme komutu (TCP için)
 #define W5100_CMD_CLOSE     0x10 // Soket kapatma komutu
 #define W5100_CMD_SEND      0x20 // Veri gönderme komutu
 #define W5100_CMD_SEND_MAC  0x21 // MAC seviyesinde gönderme komutu
 #define W5100_CMD_SEND_KEEP 0x22 // Keep-alive gönderme komutu (TCP için)
 #define W5100_CMD_RECV      0x40 // Veri alma komutu
 
 // Socket durumları - Soketin mevcut durumunu belirten değerler
 #define W5100_SOCK_CLOSED      0x00 // Soket kapalı
 #define W5100_SOCK_INIT        0x13 // Soket başlatıldı
 #define W5100_SOCK_LISTEN      0x14 // Soket dinliyor (TCP)
 #define W5100_SOCK_ESTABLISHED 0x17 // Bağlantı kuruldu (TCP)
 #define W5100_SOCK_CLOSE_WAIT  0x1C // Kapatma bekleniyor (TCP)
 #define W5100_SOCK_UDP         0x22 // UDP modu aktif
 #define W5100_SOCK_IPRAW       0x32 // IP RAW modu aktif
 #define W5100_SOCK_MACRAW      0x42 // MAC RAW modu aktif
 #define W5100_SOCK_PPPOE       0x5F // PPPoE modu aktif
 
 #define W5100_DDR  DDRB
#define W5100_PORT PORTB
#define W5100_CS   2    // Arduino UNO için pin 10 (PORTB2)

 // Fonksiyon prototipleri
 
 // W5100 başlatma - Ethernet kontrolcüsünü başlatır


 void w5100_init(void);
 
 // W5100 kapatma - Ethernet kontrolcüsünü kapatır
 void w5100_close(void);
 
 // Register okuma/yazma - Düşük seviye register işlemleri
 uint8_t w5100_read(uint16_t addr);               // Tek byte okuma
 uint16_t w5100_read_word(uint16_t addr);        // 16 bit okuma
 void w5100_read_data(uint8_t socket, uint16_t offset, uint8_t *buf, uint16_t len);

 void w5100_write(uint16_t addr, uint8_t data);   // Tek byte yazma
 void w5100_read_addr(uint16_t addr, uint8_t *buf, uint16_t len);  // Çoklu byte okuma
 void w5100_write_addr(uint16_t addr, const uint8_t *buf, uint16_t len); // Çoklu byte yazma
 
 // Socket işlemleri - Yüksek seviye soket fonksiyonları
 uint8_t w5100_socket_open(uint8_t socket, uint8_t mode, uint16_t port, uint8_t flag); // Soket açma
 void w5100_socket_close(uint8_t sock);  // Soket kapatma
 uint16_t w5100_socket_receive(uint8_t sock, uint8_t *buf, uint16_t bufsize);  // Veri alma
 void w5100_socket_send(uint8_t sock, uint8_t *buf, uint16_t bufsize);  // Veri gönderme
 
 void w5100_debug_status(void);
 uint8_t w5100_ping(uint8_t *ip_addr);

 static inline uint8_t getSn_SR(uint8_t socket) {
    return w5100_read(W5100_S0_SR + socket * 0x100);
}
 #endif /* _W5100_H_ */
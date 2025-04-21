/*
 * W5100 TFTP Bootloader - SPI İletişimi
 * Bu dosya SPI iletişimi için gerekli fonksiyonları içerir
 */

 #include <avr/io.h>
 #include "spi.h"
 
 // SPI başlatma fonksiyonu
 void spi_init(void) {
     // SPI pinlerini ayarla
     SPI_DDR |= (1 << SPI_SS) | (1 << SPI_MOSI) | (1 << SPI_SCK);  // SS, MOSI ve SCK çıkış
     SPI_DDR &= ~(1 << SPI_MISO);  // MISO giriş
     
     // SS pinini HIGH yap (devre dışı)
     SPI_PORT |= (1 << SPI_SS);
     
     // SPI'yı etkinleştir
     // SPE: SPI Enable
     // MSTR: Master Mode
     // SPR0/SPR1: SPI Clock Rate (fosc/16)
     SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
     
     // Double SPI Speed (SPI2X)
     SPSR = (1 << SPI2X);
 }
 
 void spi_deinit(void) {
    // SPI'ı devre dışı bırak
    SPCR &= ~(1 << SPE);
    
    // SPI pinlerini giriş olarak ayarla (yüksek empedans durumu)
    // SS (B2), MOSI (B3), SCK (B5) pinleri
    DDRB &= ~((1 << DDB2) | (1 << DDB3) | (1 << DDB5));
    
    // Pull-up dirençlerini devre dışı bırak
    PORTB &= ~((1 << PORTB2) | (1 << PORTB3) | (1 << PORTB5));
}

 // SPI üzerinden byte gönderme ve alma fonksiyonu
 uint8_t spi_transfer(uint8_t data) {
     // Veriyi SPI veri register'ına yaz
     SPDR = data;
     
     // Transfer tamamlanana kadar bekle
     while (!(SPSR & (1 << SPIF)));
     
     // Alınan veriyi döndür
     return SPDR;
 }
 
 // SPI chip select aktif etme fonksiyonu (CS LOW)
 void spi_enable(void) {
     SPI_PORT &= ~(1 << SPI_SS);  // SS pinini LOW yap
 }
 
 // SPI chip select devre dışı bırakma fonksiyonu (CS HIGH)
 void spi_disable(void) {
     SPI_PORT |= (1 << SPI_SS);  // SS pinini HIGH yap
 }
/*
 * W5100 TFTP Bootloader - SPI İletişimi
 * Bu dosya SPI iletişimi için gerekli tanımlamaları içerir
 */

 #ifndef _SPI_H_
 #define _SPI_H_
 
 #include <stdint.h>
 
 // SPI pin tanımlamaları (Arduino Uno için)
 #define SPI_DDR  DDRB
 #define SPI_PORT PORTB
 #define SPI_SS   PINB2   // SS   - Pin 10
 #define SPI_MOSI PINB3   // MOSI - Pin 11
 #define SPI_MISO PINB4   // MISO - Pin 12
 #define SPI_SCK  PINB5   // SCK  - Pin 13
 
 // Fonksiyon prototipleri
 
 // SPI başlatma
 void spi_init(void);
 
 void spi_deinit(void);
 // SPI üzerinden byte gönderme ve alma
 uint8_t spi_transfer(uint8_t data);
 
 // SPI chip select aktif etme (CS LOW)
 void spi_enable(void);
 
 // SPI chip select devre dışı bırakma (CS HIGH)
 void spi_disable(void);
 
 #endif /* _SPI_H_ */
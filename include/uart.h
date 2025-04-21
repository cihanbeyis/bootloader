// uart.h
#ifndef UART_H
#define UART_H

#include <avr/pgmspace.h>

// Debug modu - Seri port üzerinden debug mesajları için
// Gerçek uygulamada bu satırı yorum yaparak debug devre dışı bırakılabilir

#define DEBUG(str) debug_message(PSTR(str))

// UART baud rate (seri port hızı)
#define UART_BAUD_RATE 57600

// Fonksiyon prototipleri

// UART başlatma - Seri port iletişimi için
void uart_init(void);

void uart_deinit(void);
// UART üzerinden byte gönderme
void uart_putc(uint8_t c);

// UART üzerinden string gönderme
void uart_puts(const char *s);

void uart_putip(uint8_t* ip);

// UART üzerinden program belleğindeki string gönderme
void uart_puts_p(const char *s);
void uart_puthex(uint8_t val);
// Debug mesajı gönderme - Sadece DEBUG tanımlıysa çalışır
void debug_message(const char *msg);

// Hexadecimal değeri ASCII karaktere dönüştürme
char nibble_to_hex(uint8_t nibble);

// Byte değerini hexadecimal olarak yazdırma
void print_hex(uint8_t byte);

// Word değerini hexadecimal olarak yazdırma
void print_hex_word(uint16_t word);
void uart_printf(const char *fmt, ...);

#endif

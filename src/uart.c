// uart.c
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdarg.h>
#include <stdio.h>
#include "uart.h"

#define BAUD 57600
#define UBRR_VALUE ((F_CPU/16/BAUD) - 1)
#define PRINTF_BUF_SIZE 128

void uart_init(void) {
    // Set baud rate
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;
    
    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    
    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_deinit(void) {
    // UART kesmeleri devre dışı bırak
    UCSR0B &= ~((1 << RXCIE0) | (1 << TXCIE0) | (1 << UDRIE0));
    
    // UART'ı tamamen kapat
    UCSR0B &= ~((1 << RXEN0) | (1 << TXEN0));
    
    // Baud rate ayarlarını sıfırla
    UBRR0H = 0;
    UBRR0L = 0;
}

void uart_putc(uint8_t c) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));
    
    // Put data into buffer, sends the data
    UDR0 = c;
}

void uart_puts(const char* str) {
    while (*str) {
        // Add carriage return before newline for proper terminal display
        if (*str == '\n') {
            uart_putc('\r');
        }
        uart_putc(*str++);
    }
}

void uart_puts_p(const char *s) {
    // Program belleğindeki string sonuna kadar her karakteri gönder
    char c;
    while ((c = pgm_read_byte(s++))) {
        uart_putc(c);
    }
}

void uart_puthex(uint8_t val) {
    static const char hex[] = "0123456789ABCDEF";
    uart_putc(hex[val >> 4]);   // High nibble
    uart_putc(hex[val & 0x0F]); // Low nibble
}

void uart_putip(uint8_t* ip) {
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t num = ip[i];
        
        // Handle hundreds
        if (num >= 100) {
            uart_putc('0' + (num / 100));
            num %= 100;
        }
        
        // Handle tens
        if (num >= 10 || ip[i] >= 100) {
            uart_putc('0' + (num / 10));
            num %= 10;
        }
        
        // Handle ones
        uart_putc('0' + num);
        
        // Add dot except for last number
        if (i < 3) uart_putc('.');
    }
}

int uart_active(void) {
    uart_putc('U');
    
    // Wait a bit for echo
    for (uint16_t i = 0; i < 1000; i++) {
        if (UCSR0A & (1 << RXC0)) {
            return (UDR0 == 'U');  // Check if we received our test character back
        }
    }
    return 0;  // No response received
}


void uart_printf(const char *fmt, ...) {
    char buf[PRINTF_BUF_SIZE];
    va_list args;
    
    va_start(args, fmt);
    #ifdef __AVR__
        vsprintf_P(buf, fmt, args);  // AVR için vsprintf_P kullan
    #else
        vsnprintf(buf, sizeof(buf), fmt, args);  // Diğer platformlar için
    #endif
    va_end(args);
    
    // Buffer'daki karakterleri seri porttan gönder
    const char *p = buf;
    while (*p) {
        uart_putc(*p++);
    }
}

// Hexadecimal değeri ASCII karaktere dönüştürme fonksiyonu
char nibble_to_hex(uint8_t nibble) {
    // 0-9 için '0'-'9', 10-15 için 'A'-'F'
    nibble &= 0x0F;
    return (nibble > 9) ? (nibble - 10 + 'A') : (nibble + '0');
}

// Byte değerini hexadecimal olarak yazdırma fonksiyonu
void print_hex(uint8_t byte) {
    // Yüksek ve düşük 4 biti ayrı ayrı yazdır
    uart_putc(nibble_to_hex(byte >> 4));
    uart_putc(nibble_to_hex(byte));
}

// Word değerini hexadecimal olarak yazdırma fonksiyonu
void print_hex_word(uint16_t word) {
    // Yüksek byte ve düşük byte'ı ayrı ayrı yazdır
    print_hex(word >> 8);
    print_hex(word & 0xFF);
}

 // Debug mesajı gönderme fonksiyonu
 void debug_message(const char *msg) {
    uart_puts_p(msg);
    uart_puts_p(PSTR("\r\n"));
}

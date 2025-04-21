#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint32_t timer_count = 0;

ISR(TIMER1_COMPA_vect) {
    timer_count++;
}

void timer_init(void) {
    // Timer1'i CTC modunda 1ms kesme için ayarla
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // prescaler 64
    OCR1A = (F_CPU / 64 / 1000) - 1; // 1ms için
    TIMSK1 |= (1 << OCIE1A); // Compare Match A kesmesini etkinleştir
}

uint32_t timer_get(void) {
    uint32_t count;
    cli();
    count = timer_count;
    sei();
    return count;
}
